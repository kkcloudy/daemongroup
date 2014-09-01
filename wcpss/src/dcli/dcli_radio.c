#ifdef _D_WCPSS_
#include <string.h>
#include <stdio.h>
#include <zebra.h>
#include <dbus/dbus.h>
#include "command.h"

#include "dcli_main.h"/**wangchao changed**/

#include "dcli_radio.h"
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef1.h"        /*xm add 08/12/01*/
#include "dcli_wtp.h"
#include "wid_radio.h"
#include "asd_sta.h"
#include "sysdef/npd_sysdef.h"
#include "dbus/wcpss/dcli_wid_radio.h"    /*fengwenchao add 20110315*/
#include "bsd/bsdpub.h"
#include "dcli_wlan.h"

interface_wlan_tunnel_mode_cmd;
interface_radio_tunnel_mode_cmd;

/*fengwenchao copy from ht2.0 for requirements-407*/
typedef enum{
	dcli_mcs_check_mcsid=0,
	dcli_mcs_check_comma=1,
	dcli_mcs_check_fail=2,
	dcli_mcs_check_end=3,
	dcli_mcs_check_success=4,
	dcli_mcs_check_bar=5
}mcs_list_state;
#define MCS_LIST_SPLIT_COMMA  ','	
#define MCS_LIST_SPLIT_BAR 	  '-'	
/*fengwenchao copy end*/
/***wangchao moved to dcli_wireless_main.c****/
#if 0
struct cmd_node radio_node =
{
	RADIO_NODE,
	"%s(config-radio %d-%d)# "
};
struct cmd_node hansi_radio_node =
{
	HANSI_RADIO_NODE,
	"%s(hansi-radio %d-%d-%d-%d)# ",
	1
};
struct cmd_node local_hansi_radio_node =
{
	LOCAL_HANSI_RADIO_NODE,
	"%s(local-hansi-radio %d-%d-%d-%d)# ",
	1
};
#endif
/*zhaoruijia,20100908, 给解析函数增加 最大数值判断和非法输入报错 ,start*/

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
int free_mcs_list(update_mcs_list *mcslist){
	if (mcslist == NULL)
		return 0;
	int i = 0;
	struct tag_mcsid *tmp= NULL;
	for( i = 0; (i < mcslist->count)&&(mcslist->mcsidlist); i++ )
	{
		tmp = mcslist->mcsidlist;
		mcslist->mcsidlist = mcslist->mcsidlist->next;
		tmp->next = NULL;
		free(tmp);
		tmp = NULL;
	}
	if(mcslist){
		free(mcslist);
		mcslist = NULL;
	}
	return i+1;
}
/*fengwenchao copy from ht2.0 for requirements-407*/
int parse_mcs_list(char* ptr,update_mcs_list **mcslist)
{
	char* endPtr = NULL;
	int   mcsid1 = 0;
	int   mcsid2 = 0;
	int   min = 0;
	int	  max = 0;
	endPtr = ptr;
	mcs_list_state state = dcli_mcs_check_mcsid;
	struct tag_mcsid *mcs_id = NULL;
	
	while(1)
	{
		switch(state)
		{
			case dcli_mcs_check_mcsid: 
				mcsid1 = strtoul(endPtr,&endPtr,10);
				if(mcsid1>=0&&mcsid1<= 31)
				{
            				state=dcli_mcs_check_comma;
				}
				else
					state=dcli_mcs_check_fail;
				break;
		
			case dcli_mcs_check_comma: 
				
				if(MCS_LIST_SPLIT_COMMA == endPtr[0])
				{
					endPtr = (char*)endPtr + 1;
					state = dcli_mcs_check_mcsid;
					//save mcsid1
					mcs_id = (struct tag_mcsid*)malloc(sizeof(struct tag_mcsid));
					mcs_id->mcsid = mcsid1;
					mcs_id->next = NULL;

					//insert to list
					mcs_id->next = (*mcslist)->mcsidlist;
					(*mcslist)->mcsidlist = mcs_id;
					(*mcslist)->count++;
															
				}
				else if(MCS_LIST_SPLIT_BAR == endPtr[0])
				{
					endPtr = (char*)endPtr + 1;
					mcsid2 = strtoul(endPtr,&endPtr,10);
					if(mcsid2>=0&&mcsid2<= 31)
					{
	            		//save mcsid1
						min = (mcsid2 > mcsid1)?mcsid1:mcsid2;
						max = (mcsid2 > mcsid1)?mcsid2:mcsid1;
						while(min <= max)
						{
							mcs_id = (struct tag_mcsid*)malloc(sizeof(struct tag_mcsid));
							mcs_id->mcsid = min;
							mcs_id->next = NULL;

							//insert to list
							mcs_id->next = (*mcslist)->mcsidlist;
							(*mcslist)->mcsidlist = mcs_id;
							(*mcslist)->count++;	
								
							min++;
						}
						if('\0' == endPtr[0])
						{
							return 0;
						}
						else
						{											
							endPtr = (char*)endPtr + 1;
							state=dcli_mcs_check_mcsid;
						}
					}
					else
					{
						state = dcli_mcs_check_fail;
					}
					
				}
				else
					state = dcli_mcs_check_end;
				//printf("state2 = %d\n",state);
				//printf("mcsid = %d\n",mcsid1);
				//printf("mcsid = %d\n",mcsid2);
				break;
				
		
			case dcli_mcs_check_fail:
                //printf("state3 = %d\n",state);
				//printf("mcsid = %d\n",mcsid1);
				//printf("mcsid = %d\n",mcsid2);
				return -1;
				break;

			case dcli_mcs_check_end: 
				
				if ('\0' == endPtr[0]) 
				{
					state = dcli_mcs_check_success;
				}
				else
					state = dcli_mcs_check_fail;
			    //printf("state4 = %d\n",state);
				//printf("mcsid = %d\n",mcsid1);
				//printf("mcsid = %d\n",mcsid2);
				break;
			
			case dcli_mcs_check_success: 
				
				//save mcsid1
				mcs_id = (struct tag_mcsid*)malloc(sizeof(struct tag_mcsid));
				mcs_id->mcsid = mcsid1;
				mcs_id->next = NULL;

				//insert to list
				mcs_id->next = (*mcslist)->mcsidlist;
				(*mcslist)->mcsidlist = mcs_id;
				(*mcslist)->count++;
				//printf("state5 = %d\n",state);
				//printf("mcsid = %d\n",mcsid1);
				//printf("mcsid = %d\n",mcsid2);
				return 0;
				break;
			
			default:
                //printf("state6 = %d\n",state);
				//printf("mcsid = %d\n",mcsid1);
				//printf("mcsid = %d\n",mcsid2);
				break;
		}
		
	}
	
}
/*fengwenchao add end*/

/*zhaoruijia,20100908, 给解析函数增加 最大数值判断和非法输入报错 ,end*/

int process_rate_list(int iArray[],int num) 
{ 

	int iTemp,i,j,iCount=1; 
	/*排序 */
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
	//去重复 */
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
			
			if(*wtpid > 0 && *wtpid <= 4096){
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

int radiolist_add_node(update_radio_list *list, unsigned int radioid)
{
	struct tag_radioid *tmp = NULL, *next = NULL;

	if (!list || !radioid)
	{
		return 0;
	}

	for (next = list->radioidlist; next; next = next->next)
	{
		if (next->radioid == radioid)
		{
			return 0;
		}
	}
	
	tmp = (struct tag_radioid*)malloc(sizeof(struct tag_radioid));
	if (!tmp)
	{
		return -1;
	}
	memset(tmp, 0, sizeof(struct tag_radioid));
	tmp->radioid = radioid;
	tmp->next = NULL;

	tmp->next = list->radioidlist;
	list->radioidlist = tmp;
	list->count++;
	
	printf("add radio %d-%d (%d)\n", radioid / L_RADIO_NUM, radioid % L_RADIO_NUM, radioid);

	return 0;
}


int parse_radio_list(char *ptr, update_radio_list *list)
{
	char  *endPtr = NULL;
	unsigned int g_radioid = 0, wtpid = 0, id = 0;
	
	endPtr = ptr;
	if (!ptr)
	{
		return -1;
	}

	while (endPtr && *endPtr) 
	{
		
		id = strtoul(endPtr, &endPtr, 10);

		/* TODO */		
		if ((',' == *endPtr)) 
		{
			if (0 == wtpid)
			{
				if (id)
				{
					g_radioid = id;
					radiolist_add_node(list, g_radioid);
					wtpid = 0;
				}
			}
			else
			{
				if (id >= L_RADIO_NUM)
				{
					return -1;
				}
				g_radioid = wtpid * L_RADIO_NUM + id;
				radiolist_add_node(list, g_radioid);
				wtpid = 0;
			}
			endPtr++;
		}
		else if ('-' == *endPtr)
		{
			wtpid = id;
			endPtr++;
		}
		else if ('\0' == *endPtr)
		{
			if (0 == wtpid)
			{
				if (id)
				{
					g_radioid = id;
					radiolist_add_node(list, g_radioid);					
					wtpid = 0;
				}
			}
			else
			{
				if (id >= L_RADIO_NUM)
				{
					return -1;
				}
				g_radioid = wtpid * L_RADIO_NUM + id;
				radiolist_add_node(list, g_radioid);
				
				wtpid = 0;
			}
			break; 
		}
		else 
		{
			return -1;
		}
	}

	return 0;	
}


DEFUN(show_radio_list_cmd_func,
		 show_radio_list_cmd,
		 "show radio (list|all) [remote] [local] [PARAMETER]",
		 SHOW_STR
		 "Display Radio Information\n"
		 "Radio list summary\n"
		 "Radio list summary\n"
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
	//WID_WTP_RADIO **RADIO;	
	int ret,i=0;
	unsigned int num = 0;
	//RADIO = malloc(G_RADIO_NUM*sizeof(WID_WTP_RADIO *));
	DCLI_RADIO_API_GROUP_ONE *RADIOINFO = NULL;
	
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
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
	
#if 0	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_RADIOLIST);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_RADIOLIST);*/
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		if(RADIO)
		{
			free(RADIO);
			RADIO = NULL;
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
			
			RADIO[i] = (WID_WTP_RADIO*)malloc(sizeof(WID_WTP_RADIO));
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(RADIO[i]->WTPID));
		
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(RADIO[i]->Radio_G_ID));
		
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(RADIO[i]->Radio_Type));
		
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(RADIO[i]->Support_Rate_Count));	
			
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(RADIO[i]->FragThreshold));	
			
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(RADIO[i]->BeaconPeriod));	
			
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(RADIO[i]->IsShortPreamble));	
			
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(RADIO[i]->DTIMPeriod));	
			
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(RADIO[i]->rtsthreshold));
			
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(RADIO[i]->ShortRetry));
			
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(RADIO[i]->LongRetry));
		
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(RADIO[i]->Radio_Chan));
			
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(RADIO[i]->Radio_TXP));
			
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(RADIO[i]->AdStat));
			
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(RADIO[i]->OpStat));
					
			dbus_message_iter_next(&iter_array);
		}
	}
	
	dbus_message_unref(reply);
#endif	
	if(vty->node != VIEW_NODE){
		RADIOINFO = dcli_radio_show_api_group_one(
			index,
			0,/*"show radio (list|all)"*/
			&localid,
			&ret,
			0,
			//RADIOINFO,
			dcli_dbus_connection,
			WID_DBUS_CONF_METHOD_RADIOLIST
			);	
	    vty_out(vty,"Radio list summary\n");
	    vty_out(vty,"==============================================================================\n");
		/*vty_out(vty,"RadioID  WTPID  Channel  TXpower  Adstate  Opstate      Fragment  Beacon  Preamble  DTIM   Short/longRetry RadioRate RadioType:\n");*/
		vty_out(vty,"RadioID WTPID Channel TXpower Adstate  Opstate  SupportRateCount RadioType\n");
		if(ret == -1){
			cli_syslog_info("<error> failed get reply.\n");
		}
		if(ret == 0){
			for (i = 0; i < RADIOINFO->radio_num; i++) {
				if (RADIOINFO->RADIO[i]->Radio_Chan == 0)
				{
					if((RADIOINFO->RADIO[i]->Radio_TXP == 0)||(RADIOINFO->RADIO[i]->Radio_TXP == 100))     //fengwenchao add 20110427
					{
						vty_out(vty,"%-8d%-6dauto    auto    %-9s%-9s%-16d ",RADIOINFO->RADIO[i]->Radio_G_ID,RADIOINFO->RADIO[i]->WTPID,((RADIOINFO->RADIO[i]->AdStat== 2)?dis:en),((RADIOINFO->RADIO[i]->OpStat== 2)?dis:en),RADIOINFO->RADIO[i]->Support_Rate_Count); 							
					}
					else
					{
						vty_out(vty,"%-8d%-6dauto    %-8d%-9s%-9s%-16d ",RADIOINFO->RADIO[i]->Radio_G_ID,RADIOINFO->RADIO[i]->WTPID,RADIOINFO->RADIO[i]->Radio_TXP,((RADIOINFO->RADIO[i]->AdStat== 2)?dis:en),((RADIOINFO->RADIO[i]->OpStat== 2)?dis:en),RADIOINFO->RADIO[i]->Support_Rate_Count); 		
					}
					
						
				}
				else 
				{
					if((RADIOINFO->RADIO[i]->Radio_TXP == 0)||(RADIOINFO->RADIO[i]->Radio_TXP == 100))    //fengwenchao add 20110427
					{
						vty_out(vty,"%-8d%-6d%-8dauto    %-9s%-9s%-16d ",RADIOINFO->RADIO[i]->Radio_G_ID,RADIOINFO->RADIO[i]->WTPID,RADIOINFO->RADIO[i]->Radio_Chan,((RADIOINFO->RADIO[i]->AdStat== 2)?dis:en),((RADIOINFO->RADIO[i]->OpStat== 2)?dis:en),RADIOINFO->RADIO[i]->Support_Rate_Count); 								
					}
					else
					{
						vty_out(vty,"%-8d%-6d%-8d%-8d%-9s%-9s%-16d ",RADIOINFO->RADIO[i]->Radio_G_ID,RADIOINFO->RADIO[i]->WTPID,RADIOINFO->RADIO[i]->Radio_Chan,RADIOINFO->RADIO[i]->Radio_TXP,((RADIOINFO->RADIO[i]->AdStat== 2)?dis:en),((RADIOINFO->RADIO[i]->OpStat== 2)?dis:en),RADIOINFO->RADIO[i]->Support_Rate_Count); 					
					}
					
				}
				/*vty_out(vty,"%-8d%-6d%-8d%-8d%-9s%-9s%8.1fM/bps ",RADIO[i]->Radio_G_ID,RADIO[i]->WTPID,RADIO[i]->Radio_Chan,RADIO[i]->Radio_TXP,((RADIO[i]->AdStat== 2)?dis:en),((RADIO[i]->OpStat== 2)?dis:en),RADIO[i]->Radio_Rate/10.0); 	*/	
				
				/*vty_out(vty,"%-8d%-6d%-8d%-8d%-9s%-9s%-9d%-7d%-9d%-5d%-13d%-6d%-6d%8.1fM/bps ",RADIO[i]->Radio_G_ID,RADIO[i]->WTPID,RADIO[i]->Radio_Chan,RADIO[i]->Radio_TXP,((RADIO[i]->AdStat== 2)?dis:en),((RADIO[i]->OpStat== 2)?dis:en),RADIO[i]->FragThreshold,RADIO[i]->BeaconPeriod,RADIO[i]->IsShortPreamble,RADIO[i]->DTIMPeriod,RADIO[i]->rtsthreshold,RADIO[i]->ShortRetry,RADIO[i]->LongRetry,RADIO[i]->Radio_Rate/10.0); 	*/	
				vty_out(vty,"11");
				/*fengwenchao modify begin  20111109 for GM*/
				int flag_an = 0;int flag_gn = 0;
						
				if(((RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11A) > 0)
					&&((RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11N) > 0)
					&&((RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11AN) > 0)
					&&(!(RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11B))
					&&(!(RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11G)))
				{
					vty_out(vty,"a/an");	
				}
				else if(((RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11GN) > 0)
					&&((RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11N)> 0)
					&&((RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11G)>0)
					&&(!(RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11B))
					&&(!(RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11A)))
				{	
					vty_out(vty,"g/gn");
				}				
				else
				{
					if((RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11A) > 0)
						vty_out(vty,"a");	
					if((RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11B) > 0)
						vty_out(vty,"b");
					if((RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11G) > 0)
						vty_out(vty,"g");
					if((RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11N) > 0)
						vty_out(vty,"n");	
				}
				/*fengwenchao modify  end*/
				vty_out(vty,"\n");
				//free(RADIOINFO->RADIO[i]);
				//RADIOINFO->RADIO[i] = NULL;		
			}
			//vty_out(vty,"==============================================================================\n");
			dcli_radio_free_fun(WID_DBUS_CONF_METHOD_RADIOLIST,RADIOINFO);
		}
		vty_out(vty,"==============================================================================\n");
	//	if(RADIOINFO)
	//	{
	//		free(RADIOINFO);
	//		RADIOINFO = NULL;
	//	}
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
				RADIOINFO = dcli_radio_show_api_group_one(
					profile,
					0,/*"show radio (list|all)"*/
					&localid,
					&ret,
					0,
					dcli_dbus_connection,
					WID_DBUS_CONF_METHOD_RADIOLIST
					);	
				vty_out(vty,"Radio list summary:   hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"RadioID WTPID Channel TXpower Adstate	Opstate  SupportRateCount RadioType\n");
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				if(ret == 0){
					for (i = 0; i < RADIOINFO->radio_num; i++) {
						if (RADIOINFO->RADIO[i]->Radio_Chan == 0)
						{
							if((RADIOINFO->RADIO[i]->Radio_TXP == 0)||(RADIOINFO->RADIO[i]->Radio_TXP == 100))	   //fengwenchao add 20110427
							{
								vty_out(vty,"%-8d%-6dauto	 auto	 %-9s%-9s%-16d ",RADIOINFO->RADIO[i]->Radio_G_ID,RADIOINFO->RADIO[i]->WTPID,((RADIOINFO->RADIO[i]->AdStat== 2)?dis:en),((RADIOINFO->RADIO[i]->OpStat== 2)?dis:en),RADIOINFO->RADIO[i]->Support_Rate_Count); 							
							}
							else
							{
								vty_out(vty,"%-8d%-6dauto	 %-8d%-9s%-9s%-16d ",RADIOINFO->RADIO[i]->Radio_G_ID,RADIOINFO->RADIO[i]->WTPID,RADIOINFO->RADIO[i]->Radio_TXP,((RADIOINFO->RADIO[i]->AdStat== 2)?dis:en),((RADIOINFO->RADIO[i]->OpStat== 2)?dis:en),RADIOINFO->RADIO[i]->Support_Rate_Count);		
							}
						}
						else 
						{
							if((RADIOINFO->RADIO[i]->Radio_TXP == 0)||(RADIOINFO->RADIO[i]->Radio_TXP == 100))	  //fengwenchao add 20110427
							{
								vty_out(vty,"%-8d%-6d%-8dauto	 %-9s%-9s%-16d ",RADIOINFO->RADIO[i]->Radio_G_ID,RADIOINFO->RADIO[i]->WTPID,RADIOINFO->RADIO[i]->Radio_Chan,((RADIOINFO->RADIO[i]->AdStat== 2)?dis:en),((RADIOINFO->RADIO[i]->OpStat== 2)?dis:en),RADIOINFO->RADIO[i]->Support_Rate_Count); 								
							}
							else
							{
								vty_out(vty,"%-8d%-6d%-8d	%-8d%-9s%-9s%-16d ",RADIOINFO->RADIO[i]->Radio_G_ID,RADIOINFO->RADIO[i]->WTPID,RADIOINFO->RADIO[i]->Radio_Chan,RADIOINFO->RADIO[i]->Radio_TXP,((RADIOINFO->RADIO[i]->AdStat== 2)?dis:en),((RADIOINFO->RADIO[i]->OpStat== 2)?dis:en),RADIOINFO->RADIO[i]->Support_Rate_Count);					
							}
						}
						vty_out(vty,"11");
						/*fengwenchao modify begin	20111109 for GM*/
						int flag_an = 0;int flag_gn = 0;
								
						if(((RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11A) > 0)
							&&((RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11N) > 0)
							&&((RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11AN) > 0)
							&&(!(RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11B))
							&&(!(RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11G)))
						{
							vty_out(vty,"a/an");	
						}
						else if(((RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11GN) > 0)
							&&((RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11N)> 0)
							&&((RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11G)>0)
							&&(!(RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11B))
							&&(!(RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11A)))
						{	
							vty_out(vty,"g/gn");
						}				
						else
						{
							if((RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11A) > 0)
								vty_out(vty,"a");	
							if((RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11B) > 0)
								vty_out(vty,"b");
							if((RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11G) > 0)
								vty_out(vty,"g");
							if((RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11N) > 0)
								vty_out(vty,"n");	
						}
						/*fengwenchao modify  end*/
						vty_out(vty,"\n");
					}
					dcli_radio_free_fun(WID_DBUS_CONF_METHOD_RADIOLIST,RADIOINFO);
				}
				vty_out(vty,"==================================================================================\n");
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
				RADIOINFO = dcli_radio_show_api_group_one(
					profile,
					0,/*"show radio (list|all)"*/
					&localid,
					&ret,
					0,
					dcli_dbus_connection,
					WID_DBUS_CONF_METHOD_RADIOLIST
					);	
				vty_out(vty,"Radio list summary:   local-hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"RadioID WTPID Channel TXpower Adstate	Opstate  SupportRateCount RadioType\n");
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				if(ret == 0){
					for (i = 0; i < RADIOINFO->radio_num; i++) {
						if (RADIOINFO->RADIO[i]->Radio_Chan == 0)
						{
							if((RADIOINFO->RADIO[i]->Radio_TXP == 0)||(RADIOINFO->RADIO[i]->Radio_TXP == 100))	   //fengwenchao add 20110427
							{
								vty_out(vty,"%-8d%-6dauto	 auto	 %-9s%-9s%-16d ",RADIOINFO->RADIO[i]->Radio_G_ID,RADIOINFO->RADIO[i]->WTPID,((RADIOINFO->RADIO[i]->AdStat== 2)?dis:en),((RADIOINFO->RADIO[i]->OpStat== 2)?dis:en),RADIOINFO->RADIO[i]->Support_Rate_Count); 							
							}
							else
							{
								vty_out(vty,"%-8d%-6dauto	 %-8d%-9s%-9s%-16d ",RADIOINFO->RADIO[i]->Radio_G_ID,RADIOINFO->RADIO[i]->WTPID,RADIOINFO->RADIO[i]->Radio_TXP,((RADIOINFO->RADIO[i]->AdStat== 2)?dis:en),((RADIOINFO->RADIO[i]->OpStat== 2)?dis:en),RADIOINFO->RADIO[i]->Support_Rate_Count);		
							}
						}
						else 
						{
							if((RADIOINFO->RADIO[i]->Radio_TXP == 0)||(RADIOINFO->RADIO[i]->Radio_TXP == 100))	  //fengwenchao add 20110427
							{
								vty_out(vty,"%-8d%-6d%-8dauto	 %-9s%-9s%-16d ",RADIOINFO->RADIO[i]->Radio_G_ID,RADIOINFO->RADIO[i]->WTPID,RADIOINFO->RADIO[i]->Radio_Chan,((RADIOINFO->RADIO[i]->AdStat== 2)?dis:en),((RADIOINFO->RADIO[i]->OpStat== 2)?dis:en),RADIOINFO->RADIO[i]->Support_Rate_Count); 								
							}
							else
							{
								vty_out(vty,"%-8d%-6d%-8d	%-8d%-9s%-9s%-16d ",RADIOINFO->RADIO[i]->Radio_G_ID,RADIOINFO->RADIO[i]->WTPID,RADIOINFO->RADIO[i]->Radio_Chan,RADIOINFO->RADIO[i]->Radio_TXP,((RADIOINFO->RADIO[i]->AdStat== 2)?dis:en),((RADIOINFO->RADIO[i]->OpStat== 2)?dis:en),RADIOINFO->RADIO[i]->Support_Rate_Count);					
							}
						}
						vty_out(vty,"11");
						/*fengwenchao modify begin	20111109 for GM*/
						int flag_an = 0;int flag_gn = 0;
								
						if(((RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11A) > 0)
							&&((RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11N) > 0)
							&&((RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11AN) > 0)
							&&(!(RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11B))
							&&(!(RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11G)))
						{
							vty_out(vty,"a/an");	
						}
						else if(((RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11GN) > 0)
							&&((RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11N)> 0)
							&&((RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11G)>0)
							&&(!(RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11B))
							&&(!(RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11A)))
						{	
							vty_out(vty,"g/gn");
						}				
						else
						{
							if((RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11A) > 0)
								vty_out(vty,"a");	
							if((RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11B) > 0)
								vty_out(vty,"b");
							if((RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11G) > 0)
								vty_out(vty,"g");
							if((RADIOINFO->RADIO[i]->Radio_Type&IEEE80211_11N) > 0)
								vty_out(vty,"n");	
						}
						/*fengwenchao modify  end*/
						vty_out(vty,"\n");
					}
					dcli_radio_free_fun(WID_DBUS_CONF_METHOD_RADIOLIST,RADIOINFO);
				}
				vty_out(vty,"==================================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}
	}

	return CMD_SUCCESS;
}


DEFUN(show_radio_cmd_func,
		 show_radio_cmd,
		 "show radio RADIOID [remote] [local] [PARAMETER]",
		 SHOW_STR
		 "Display Radio Information\n"
		 "Global Radio ID\n"
		 "'remote' or 'local' hansi\n"
		 "'remote' or 'local' hansi\n"
		 "slotid-instid\n"
)
{
	unsigned int radio_id;
	unsigned char num;
	WID_WTP_RADIO *radio;
	char dis[] = "disable";
	char en[] = "enable";
	char whichinterface[RADIO_IF_NAME_LEN];/*sz20080825*/
	unsigned char wlanid[WLAN_NUM] = {0}; /*should make */
	int wlannum = 0;
	int i=0;
	int ret=0;
	int ratelist[20];
/*	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;*/
	unsigned int bss_max_sta[L_BSS_NUM];
	char wds[20];
/*    radio_id = atoi(argv[0]);*/
	int jj=0;
	int wtpid,l_radioid;
	unsigned char auto_channel_state;
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	struct wlanid  *tmp = NULL;

	ret = parse_radio_id((char*)argv[0],&wtpid,&l_radioid);

	if (ret != WID_DBUS_SUCCESS) 
	{
		ret = parse_int_ID((char*)argv[0], &radio_id);
		
		if(ret != WID_DBUS_SUCCESS){

			vty_out(vty,"<error> error parameter format correct format is wtpid-radioid [1-0][3-1]\n");
			
			return CMD_WARNING;
		}
	}
	else
	{	
		radio_id = wtpid*L_RADIO_NUM + l_radioid;
	}
	
	//ret = parse_int_ID((char*)argv[0], &radio_id);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}
	if(radio_id > G_RADIO_NUM){
		vty_out(vty,"<error> radio id should be 1 to %d\n",G_RADIO_NUM);
		return CMD_SUCCESS;
	}
	DCLI_RADIO_API_GROUP_ONE *RADIOINFO = NULL;
	
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
	
#if 0	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOWRADIO);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SHOWRADIO);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
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
			
			radio = (WID_WTP_RADIO*)malloc(sizeof(WID_WTP_RADIO));
			/*
			for(jj=0;jj<4;jj++){
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&bss_max_sta[jj]);//xm
			}
			*/
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radio->WTPID);	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radio->Radio_L_ID);	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radio->Radio_G_ID);	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radio->Radio_Type);	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radio->Support_Rate_Count);	

			for (i=0;i<(radio->Support_Rate_Count);i++)
			{
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&ratelist[i]);
			}
			#if TEST_SWITCH_WAY

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wlannum); 
			
			for (i = 0; i < wlannum; i++)
			{
				dbus_message_iter_next(&iter);				
				dbus_message_iter_get_basic(&iter,&wlanid[i]);
				
			}
			#endif
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radio->FragThreshold);	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radio->BeaconPeriod);	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radio->IsShortPreamble);	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radio->DTIMPeriod);	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radio->rtsthreshold);	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radio->ShortRetry);
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radio->LongRetry);
						
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radio->Radio_Chan);	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radio->Radio_TXP);	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radio->AdStat);	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radio->OpStat);	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radio->upcount);	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radio->downcount);				

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&auto_channel_state);	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&num);	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_recurse(&iter,&iter_array);
			
			for (i = 0; i < num; i++) {
				DBusMessageIter iter_struct;
				
				radio->BSS[i]= (WID_BSS*)malloc(sizeof(WID_BSS));
				radio->BSS[i]->BSSID = (unsigned char *)malloc(6);
				memset(radio->BSS[i]->BSSID,0,6);
				dbus_message_iter_recurse(&iter_array,&iter_struct);

				dbus_message_iter_get_basic(&iter_struct,&(radio->BSS[i]->bss_max_allowed_sta_num));
			
				dbus_message_iter_next(&iter_struct);

				dbus_message_iter_get_basic(&iter_struct,&(radio->BSS[i]->WlanID));
			
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(radio->BSS[i]->Radio_G_ID));
			
				dbus_message_iter_next(&iter_struct);
		
				dbus_message_iter_get_basic(&iter_struct,&(radio->BSS[i]->BSSID[0]));
			
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(radio->BSS[i]->BSSID[1]));
			
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(radio->BSS[i]->BSSID[2]));
			
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(radio->BSS[i]->BSSID[3]));
			
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(radio->BSS[i]->BSSID[4]));
			
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(radio->BSS[i]->BSSID[5]));
			
				dbus_message_iter_next(&iter_struct);
			
				dbus_message_iter_get_basic(&iter_struct,&(radio->BSS[i]->State));
				
				dbus_message_iter_next(&iter_struct);
			
				dbus_message_iter_get_basic(&iter_struct,&(radio->BSS[i]->BSS_IF_POLICY));

				dbus_message_iter_next(&iter_struct);
			
				dbus_message_iter_get_basic(&iter_struct,&(radio->BSS[i]->keyindex));/*sz add 05-30*/

				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(radio->BSS[i]->upcount));

				dbus_message_iter_next(&iter_struct);	
				
				dbus_message_iter_get_basic(&iter_struct,&(radio->BSS[i]->downcount));

				dbus_message_iter_next(&iter_struct);				
			
				dbus_message_iter_get_basic(&iter_struct,&(radio->BSS[i]->WDSStat));
						
				dbus_message_iter_next(&iter_array);
			}
		}
		
		dbus_message_unref(reply);
#endif
	if(vty->node != VIEW_NODE){
		RADIOINFO = dcli_radio_show_api_group_one(
			index,
			radio_id,/*"show radio RADIOID"*/
			&localid,
			&ret,
			0,
			//RADIOINFO,
			dcli_dbus_connection,
			WID_DBUS_CONF_METHOD_SHOWRADIO
			);	
		if(ret == -1){
			cli_syslog_info("<error> failed get reply.\n");
		}
		if(ret == 0){	
			vty_out(vty,"==============================================================================\n");
			/*vty_out(vty,"RadioID:	%d\nWTPID:		%d\nRadio_local_ID:	%d\nChannel:	%d\nTX power:	%d\nAdstate:	%s\nOpstate:	%s\nRadio Rate:   	%0.1fM/bps\nFragThreshold:  %d\nBeaconPeriod:   %d\nShortPreamble:  %d\nDTIMPeriod:     %d\nRTSThreshold:   %d\nShortRetry:     %d\nLongRetry:      %d\n",\*/
			/*	radio_id,radio->WTPID,radio->Radio_L_ID,radio->Radio_Chan,radio->Radio_TXP,((radio->AdStat== 2)?dis:en),((radio->OpStat== 2)?dis:en),radio->Radio_Rate/10.0,radio->FragThreshold,radio->BeaconPeriod,radio->IsShortPreamble,radio->DTIMPeriod,radio->rtsthreshold,radio->ShortRetry,radio->LongRetry);	*/		
			vty_out(vty,"RadioID:	%d\nWTPID:		%d\nRadio_local_ID:	%d\nAdstate:	%s\nOpstate:	%s\nFragThreshold:  %d\nBeaconPeriod:   %d\nShortPreamble:  %d\nDTIMPeriod:     %d\nRTSThreshold:   %d\nShortRetry:     %d\nLongRetry:      %d\n",\
				radio_id,RADIOINFO->RADIO[0]->WTPID,RADIOINFO->RADIO[0]->Radio_L_ID,((RADIOINFO->RADIO[0]->AdStat== 2)?dis:en),((RADIOINFO->RADIO[0]->OpStat== 2)?dis:en),RADIOINFO->RADIO[0]->FragThreshold,RADIOINFO->RADIO[0]->BeaconPeriod,RADIOINFO->RADIO[0]->IsShortPreamble,RADIOINFO->RADIO[0]->DTIMPeriod,RADIOINFO->RADIO[0]->rtsthreshold,RADIOINFO->RADIO[0]->ShortRetry,RADIOINFO->RADIO[0]->LongRetry);			
			if((RADIOINFO->RADIO[0]->Radio_TXP == 0)||((RADIOINFO->RADIO[0]->Radio_TXP == 100)))
			{
				vty_out(vty,"txpower:	auto\n");
			}
			else 
			{
				vty_out(vty,"txpower:	%d\n",RADIOINFO->RADIO[0]->Radio_TXP);
			}
			
			if (RADIOINFO->RADIO[0]->txpowerautostate == 0)
			{
				vty_out(vty,"txpower state:	auto\n");
			}
			else if(RADIOINFO->RADIO[0]->txpowerautostate == 1)
			{
				vty_out(vty,"txpower state:	manual\n");
			}
			vty_out(vty,"txpowerstep:  %d\n",RADIOINFO->RADIO[0]->txpowerstep);   //fengwenchao add 20110329
			vty_out(vty,"uptime:         %d\n",RADIOINFO->RADIO[0]->upcount);
			vty_out(vty,"downtime:       %d\n",RADIOINFO->RADIO[0]->downcount);
			
			if (RADIOINFO->RADIO[0]->Radio_Chan == 0)
			{
				vty_out(vty,"Channel:	auto\n");
			}
			else 
			{
				vty_out(vty,"Channel:	%d\n",RADIOINFO->RADIO[0]->Radio_Chan);
			}
			if(RADIOINFO->RADIO[0]->auto_channel_cont ==0){
				vty_out(vty,"channel state : auto\n");
			}
			else {
				vty_out(vty,"channel state : manual\n");
			}
			vty_out(vty,"SupportRateCount:	%d\n",RADIOINFO->RADIO[0]->Support_Rate_Count);
			
			if (RADIOINFO->RADIO[0]->Support_Rate_Count != 0)
			{
				vty_out(vty,"Support Rate:   ");
					for (i=0;i<(RADIOINFO->RADIO[0]->Support_Rate_Count);i++)
					{	
						
							vty_out(vty,"%0.1f ",(*(RADIOINFO->RADIO[0]->RadioRate[i])) /10.0);
				}
				vty_out(vty,"M/bps\n");
			}
			
			#if TEST_SWITCH_WAY
			
			vty_out(vty,"radio apply wlan id: ");
				for (i = 0; i < RADIOINFO->wlan_num; i++)
				{	
						vty_out(vty,"%d  ",(RADIOINFO->RADIO[0]->WlanId[i]));
			}
			if(i == 0)
			{
				vty_out(vty,"NONE\n");
			}
			else
			{
				vty_out(vty,"\n");
			}	
			
			#endif
			
			vty_out(vty,"Radio Type:	");
			/*fengwenchao modify begin  20111109 for GM*/
			int flag_an = 0;int flag_gn = 0;
			if(RADIOINFO->RADIO[0]->Radio_Type != 0){				
				vty_out(vty,"11");			

				if(((RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11A) > 0)
					&&((RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11N) > 0)
					&&((RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11AN) > 0)
					&&(!(RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11B))
					&&(!(RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11G)))
				{
					vty_out(vty,"a/an");
				}				
				else if(((RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11GN) > 0)
					&&((RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11N)> 0)
					&&((RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11G)>0)
					&&(!(RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11B))
					&&(!(RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11A)))
				{	
					vty_out(vty,"g/gn");
				}	
				else
				{
					if((RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11A) > 0)
						vty_out(vty,"a");
					if((RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11B) > 0)
						vty_out(vty,"b");
					if((RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11G) > 0)
						vty_out(vty,"g");
					if((RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11N) > 0)
						vty_out(vty,"n");	
				}
			}
			/*fengwenchao modify  end*/
			vty_out(vty,"\n");
			if((RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11N)>0){
			if(RADIOINFO->RADIO[0]->channel_offset == 0)
				vty_out(vty,"Channel offset:	%s\n","none");
			if(RADIOINFO->RADIO[0]->channel_offset == -1)
				vty_out(vty,"Channel offset:	%s\n","down");
			if(RADIOINFO->RADIO[0]->channel_offset == 1)
				vty_out(vty,"Channel offset:	%s\n","up");

            vty_out(vty,"Chainmask Number:  %d\n",RADIOINFO->RADIO[0]->chainmask_num);
			vty_out(vty,"Tx_chainmask:   ");
			if((RADIOINFO->RADIO[0]->tx_chainmask_state_value & 0x4)>0)
				vty_out(vty,"1:");
			else
				vty_out(vty,"0:");
			if((RADIOINFO->RADIO[0]->tx_chainmask_state_value & 0x2)>0)
				vty_out(vty,"1:");
			else
				vty_out(vty,"0:");
			if((RADIOINFO->RADIO[0]->tx_chainmask_state_value & 0x1)>0)
				vty_out(vty,"1\n");
			else
				vty_out(vty,"0\n");

		    /* zhangshu add for show Rx_chainmask, 2010-10-09 */
		    vty_out(vty,"Rx_chainmask:   ");
			if((RADIOINFO->RADIO[0]->rx_chainmask_state_value & 0x4)>0)
				vty_out(vty,"1:");
			else
				vty_out(vty,"0:");
			if((RADIOINFO->RADIO[0]->rx_chainmask_state_value & 0x2)>0)
				vty_out(vty,"1:");
			else
				vty_out(vty,"0:");
			if((RADIOINFO->RADIO[0]->rx_chainmask_state_value & 0x1)>0)
				vty_out(vty,"1\n");
			else
				vty_out(vty,"0\n");

			vty_out(vty,"ampdu state:	%s\n",(RADIOINFO->RADIO[0]->Ampdu.Able == 1)?"enable":"disable");
			vty_out(vty,"ampdu limit:	%u\n",RADIOINFO->RADIO[0]->Ampdu.AmpduLimit);

			/* zhangshu add for ampdu & amsdu show, 2010-10-09 */
			vty_out(vty,"ampdu subframe: %d\n",RADIOINFO->RADIO[0]->Ampdu.subframe);
			vty_out(vty,"amsdu state:	%s\n",(RADIOINFO->RADIO[0]->Amsdu.Able == 1)?"enable":"disable");
			vty_out(vty,"amsdu limit:	%u\n",RADIOINFO->RADIO[0]->Amsdu.AmsduLimit);
			vty_out(vty,"amsdu subframe: %d\n",RADIOINFO->RADIO[0]->Amsdu.subframe);
			vty_out(vty,"pureN Mixed:	%s\n",(RADIOINFO->RADIO[0]->MixedGreenfield.Mixed_Greenfield == 1)?"pureN":"Mixed");

			vty_out(vty,"guard interval:	%s\n",(RADIOINFO->RADIO[0]->guardinterval == 1)?"400ns":"800ns");

			if(RADIOINFO->RADIO[0]->cwmode == 0){
				vty_out(vty,"cwmode:	ht20\n");
			}
			else if(RADIOINFO->RADIO[0]->cwmode == 1){
				vty_out(vty,"cwmode:	ht20/40\n");
			}
			else if(RADIOINFO->RADIO[0]->cwmode == 2){
				vty_out(vty,"cwmode:	ht40\n");
				}
			
			//vty_out(vty,"mcs count %d\n ",RADIOINFO->RADIO[0]->mcs_count);//qiuchen change
			vty_out(vty,"mcs: ");//qiuchen change
			for(i=0;i<RADIOINFO->RADIO[0]->mcs_count;i++){
				vty_out(vty,"%d, ",RADIOINFO->RADIO[0]->mcs_list[i]);
			}
			vty_out(vty,"\n");

			
			}
			/*wcl add for OSDEVTDPB-31*/
			switch(RADIOINFO->RADIO[0]->Radio_country_code){
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
					vty_out(vty,"country-code : %d\n",RADIOINFO->RADIO[0]->Radio_country_code);
						break;
			}
			/*end*/

			vty_out(vty,"radio %d-%d BSS summary\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM);
			vty_out(vty,"==============================================================================\n");
			/*vty_out(vty,"%-6s	%-7s	%-17s  %-9s	%-16s %-16s\n","WLANID","RadioID","BSSID","BSS State","BSSPolicy","MaxStaNum");*/
			vty_out(vty,"%-6s	%-8s %-17s  %-9s	%-9s %-9s %-8s\n","WLANID","Keyindex","BSSID","BSS State","BSSPolicy","MaxStaNum","WDS_Mode");
			for (i = 0; i < RADIOINFO->bss_num; i++) {
				memset(whichinterface,0,RADIO_IF_NAME_LEN);
				CheckWIDIfPolicy(whichinterface,RADIOINFO->RADIO[0]->BSS[i]->BSS_IF_POLICY);/*sz20080825*/
				memset(wds,0,20);
				if(RADIOINFO->RADIO[0]->BSS[i]->WDSStat == 0){
					strcpy(wds,"disable");
				}else if(RADIOINFO->RADIO[0]->BSS[i]->WDSStat == 1){
					strcpy(wds,"ANY");
				}else if(RADIOINFO->RADIO[0]->BSS[i]->WDSStat == 2){
					strcpy(wds,"SOME");
				}
				vty_out(vty,"%-6d	%-8d %02X:%02X:%02X:%02X:%02X:%02X  %-9s	%-9s %-9d %-8s\n",RADIOINFO->RADIO[0]->BSS[i]->WlanID,RADIOINFO->RADIO[0]->BSS[i]->keyindex,RADIOINFO->RADIO[0]->BSS[i]->BSSID[0],RADIOINFO->RADIO[0]->BSS[i]->BSSID[1],RADIOINFO->RADIO[0]->BSS[i]->BSSID[2],RADIOINFO->RADIO[0]->BSS[i]->BSSID[3],RADIOINFO->RADIO[0]->BSS[i]->BSSID[4],
						RADIOINFO->RADIO[0]->BSS[i]->BSSID[5],((RADIOINFO->RADIO[0]->BSS[i]->State== 0)?dis:en),whichinterface,RADIOINFO->RADIO[0]->BSS[i]->bss_max_allowed_sta_num,
						wds);
				//free(RADIOINFO->RADIO[0]->BSS[i]->BSSID);
				//free(RADIOINFO->RADIO[0]->BSS[i]);
			}
			
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"radio apply wlan summary\n");
			vty_out(vty,"==============================================================================\n");
			/*vty_out(vty,"%-6s	%-7s	%-17s  %-9s	%-16s %-16s\n","WLANID","RadioID","BSSID","BSS State","BSSPolicy","MaxStaNum");*/
			vty_out(vty,"%-6s	%-8s\n","WLANID","ESSID");
			tmp = RADIOINFO->RADIO[0]->Wlan_Id;
			while(tmp != NULL)
			{
				vty_out(vty,"%-6d	%-32s\n",tmp->wlanid,tmp->ESSID);
				tmp = tmp->next;
			}
			
			vty_out(vty,"==============================================================================\n");
			dcli_radio_free_fun(WID_DBUS_CONF_METHOD_SHOWRADIO,RADIOINFO);
				//vty_out(vty,"BSS %d max sta num %d\n",jj+1,bss_max_sta[jj]);//xm*/
		}else if (ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio id does not exist\n");
		else if(ret == RADIO_ID_LARGE_THAN_MAX)		
			vty_out(vty,"<error> radio id should be 1 to %d\n",G_RADIO_NUM);
	}

	if(vty->node == VIEW_NODE){
		//for remote hansi info
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}

	hansi_parameter:
				RADIOINFO = dcli_radio_show_api_group_one(
					profile,
					radio_id,/*"show radio RADIOID"*/
					&localid,
					&ret,
					0,
					//RADIOINFO,
					dcli_dbus_connection,
					WID_DBUS_CONF_METHOD_SHOWRADIO
					);	
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"----------------------------------------------------------------------------\n");
				if(ret == 0){	
					vty_out(vty,"RadioID:	%d\nWTPID:		%d\nRadio_local_ID:	%d\nAdstate:	%s\nOpstate:	%s\nFragThreshold:  %d\nBeaconPeriod:   %d\nShortPreamble:  %d\nDTIMPeriod:     %d\nRTSThreshold:   %d\nShortRetry:     %d\nLongRetry:      %d\n",\
						radio_id,RADIOINFO->RADIO[0]->WTPID,RADIOINFO->RADIO[0]->Radio_L_ID,((RADIOINFO->RADIO[0]->AdStat== 2)?dis:en),((RADIOINFO->RADIO[0]->OpStat== 2)?dis:en),RADIOINFO->RADIO[0]->FragThreshold,RADIOINFO->RADIO[0]->BeaconPeriod,RADIOINFO->RADIO[0]->IsShortPreamble,RADIOINFO->RADIO[0]->DTIMPeriod,RADIOINFO->RADIO[0]->rtsthreshold,RADIOINFO->RADIO[0]->ShortRetry,RADIOINFO->RADIO[0]->LongRetry);			
					if((RADIOINFO->RADIO[0]->Radio_TXP == 0)||((RADIOINFO->RADIO[0]->Radio_TXP == 100)))
					{
						vty_out(vty,"txpower:	auto\n");
					}
					else 
					{
						vty_out(vty,"txpower:	%d\n",RADIOINFO->RADIO[0]->Radio_TXP);
					}
					
					if (RADIOINFO->RADIO[0]->txpowerautostate == 0)
					{
						vty_out(vty,"txpower state:	auto\n");
					}
					else if(RADIOINFO->RADIO[0]->txpowerautostate == 1)
					{
						vty_out(vty,"txpower state:	manual\n");
					}
					vty_out(vty,"txpowerstep:  %d\n",RADIOINFO->RADIO[0]->txpowerstep);   //fengwenchao add 20110329
					vty_out(vty,"uptime:         %d\n",RADIOINFO->RADIO[0]->upcount);
					vty_out(vty,"downtime:       %d\n",RADIOINFO->RADIO[0]->downcount);
					
					if (RADIOINFO->RADIO[0]->Radio_Chan == 0)
					{
						vty_out(vty,"Channel:	auto\n");
					}
					else 
					{
						vty_out(vty,"Channel:	%d\n",RADIOINFO->RADIO[0]->Radio_Chan);
					}
					if(RADIOINFO->RADIO[0]->auto_channel_cont ==0){
						vty_out(vty,"channel state : auto\n");
					}
					else {
						vty_out(vty,"channel state : manual\n");
					}
					vty_out(vty,"SupportRateCount:	%d\n",RADIOINFO->RADIO[0]->Support_Rate_Count);
					
					if (RADIOINFO->RADIO[0]->Support_Rate_Count != 0)
					{
						vty_out(vty,"Support Rate:   ");
							for (i=0;i<(RADIOINFO->RADIO[0]->Support_Rate_Count);i++)
							{	
								
									vty_out(vty,"%0.1f ",(*(RADIOINFO->RADIO[0]->RadioRate[i])) /10.0);
						}
						vty_out(vty,"M/bps\n");
					}
					
			#if TEST_SWITCH_WAY
					
					vty_out(vty,"radio apply wlan id: ");
						for (i = 0; i < RADIOINFO->wlan_num; i++)
						{	
								vty_out(vty,"%d  ",(RADIOINFO->RADIO[0]->WlanId[i]));
					}
					if(i == 0)
					{
						vty_out(vty,"NONE\n");
					}
					else
					{
						vty_out(vty,"\n");
					}	
					
			#endif
					
					vty_out(vty,"Radio Type:	");
					/*fengwenchao modify begin  20111109 for GM*/
					int flag_an = 0;int flag_gn = 0;
					if(RADIOINFO->RADIO[0]->Radio_Type != 0){				
						vty_out(vty,"11");			

						if(((RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11A) > 0)
							&&((RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11N) > 0)
							&&((RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11AN) > 0)
							&&(!(RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11B))
							&&(!(RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11G)))
						{
							vty_out(vty,"a/an");
						}				
						else if(((RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11GN) > 0)
							&&((RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11N)> 0)
							&&((RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11G)>0)
							&&(!(RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11B))
							&&(!(RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11A)))
						{	
							vty_out(vty,"g/gn");
						}	
						else
						{
							if((RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11A) > 0)
								vty_out(vty,"a");
							if((RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11B) > 0)
								vty_out(vty,"b");
							if((RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11G) > 0)
								vty_out(vty,"g");
							if((RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11N) > 0)
								vty_out(vty,"n");	
						}
					}
					/*fengwenchao modify  end*/
					vty_out(vty,"\n");
					if((RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11N)>0){
					if(RADIOINFO->RADIO[0]->channel_offset == 0)
						vty_out(vty,"Channel offset:	%s\n","none");
					if(RADIOINFO->RADIO[0]->channel_offset == -1)
						vty_out(vty,"Channel offset:	%s\n","down");
					if(RADIOINFO->RADIO[0]->channel_offset == 1)
						vty_out(vty,"Channel offset:	%s\n","up");

		            vty_out(vty,"Chainmask Number:  %d\n",RADIOINFO->RADIO[0]->chainmask_num);
					vty_out(vty,"Tx_chainmask:   ");
					if((RADIOINFO->RADIO[0]->tx_chainmask_state_value & 0x4)>0)
						vty_out(vty,"1:");
					else
						vty_out(vty,"0:");
					if((RADIOINFO->RADIO[0]->tx_chainmask_state_value & 0x2)>0)
						vty_out(vty,"1:");
					else
						vty_out(vty,"0:");
					if((RADIOINFO->RADIO[0]->tx_chainmask_state_value & 0x1)>0)
						vty_out(vty,"1\n");
					else
						vty_out(vty,"0\n");

				    /* zhangshu add for show Rx_chainmask, 2010-10-09 */
				    vty_out(vty,"Rx_chainmask:   ");
					if((RADIOINFO->RADIO[0]->rx_chainmask_state_value & 0x4)>0)
						vty_out(vty,"1:");
					else
						vty_out(vty,"0:");
					if((RADIOINFO->RADIO[0]->rx_chainmask_state_value & 0x2)>0)
						vty_out(vty,"1:");
					else
						vty_out(vty,"0:");
					if((RADIOINFO->RADIO[0]->rx_chainmask_state_value & 0x1)>0)
						vty_out(vty,"1\n");
					else
						vty_out(vty,"0\n");

					vty_out(vty,"ampdu state:	%s\n",(RADIOINFO->RADIO[0]->Ampdu.Able == 1)?"enable":"disable");
					vty_out(vty,"ampdu limit:	%u\n",RADIOINFO->RADIO[0]->Ampdu.AmpduLimit);

					/* zhangshu add for ampdu & amsdu show, 2010-10-09 */
					vty_out(vty,"ampdu subframe: %d\n",RADIOINFO->RADIO[0]->Ampdu.subframe);
					vty_out(vty,"amsdu state:	%s\n",(RADIOINFO->RADIO[0]->Amsdu.Able == 1)?"enable":"disable");
					vty_out(vty,"amsdu limit:	%u\n",RADIOINFO->RADIO[0]->Amsdu.AmsduLimit);
					vty_out(vty,"amsdu subframe: %d\n",RADIOINFO->RADIO[0]->Amsdu.subframe);
					vty_out(vty,"pureN Mixed:	%s\n",(RADIOINFO->RADIO[0]->MixedGreenfield.Mixed_Greenfield == 1)?"pureN":"Mixed");

					vty_out(vty,"guard interval:	%s\n",(RADIOINFO->RADIO[0]->guardinterval == 1)?"400ns":"800ns");

					if(RADIOINFO->RADIO[0]->cwmode == 0){
						vty_out(vty,"cwmode:	ht20\n");
					}
					else if(RADIOINFO->RADIO[0]->cwmode == 1){
						vty_out(vty,"cwmode:	ht20/40\n");
					}
					else if(RADIOINFO->RADIO[0]->cwmode == 2){
						vty_out(vty,"cwmode:	ht40\n");
						}
					
					vty_out(vty,"mcs count %d\n ",RADIOINFO->RADIO[0]->mcs_count);//qiuchen change
					vty_out(vty,"mcs: ");//qiuchen change
					for(i=0;i<RADIOINFO->RADIO[0]->mcs_count;i++){
						vty_out(vty,"%d, ",RADIOINFO->RADIO[0]->mcs_list[i]);
					}
					vty_out(vty,"\n");
					
					}
					/*wcl add for OSDEVTDPB-31*/
					switch(RADIOINFO->RADIO[0]->Radio_country_code){
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
							vty_out(vty,"country-code : %d\n",RADIOINFO->RADIO[0]->Radio_country_code);
								break;
					}
					/*end*/

					vty_out(vty,"radio %d-%d BSS summary\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM);
					vty_out(vty,"-------------------------------------------------------------------------------\n");
					vty_out(vty,"%-6s	%-8s %-17s  %-9s	%-9s %-9s %-8s\n","WLANID","Keyindex","BSSID","BSS State","BSSPolicy","MaxStaNum","WDS_Mode");
					for (i = 0; i < RADIOINFO->bss_num; i++) {
						memset(whichinterface,0,RADIO_IF_NAME_LEN);
						CheckWIDIfPolicy(whichinterface,RADIOINFO->RADIO[0]->BSS[i]->BSS_IF_POLICY);/*sz20080825*/
						memset(wds,0,20);
						if(RADIOINFO->RADIO[0]->BSS[i]->WDSStat == 0){
							strcpy(wds,"disable");
						}else if(RADIOINFO->RADIO[0]->BSS[i]->WDSStat == 1){
							strcpy(wds,"ANY");
						}else if(RADIOINFO->RADIO[0]->BSS[i]->WDSStat == 2){
							strcpy(wds,"SOME");
						}
						vty_out(vty,"%-6d	%-8d %02X:%02X:%02X:%02X:%02X:%02X  %-9s	%-9s %-9d %-8s\n",RADIOINFO->RADIO[0]->BSS[i]->WlanID,RADIOINFO->RADIO[0]->BSS[i]->keyindex,RADIOINFO->RADIO[0]->BSS[i]->BSSID[0],RADIOINFO->RADIO[0]->BSS[i]->BSSID[1],RADIOINFO->RADIO[0]->BSS[i]->BSSID[2],RADIOINFO->RADIO[0]->BSS[i]->BSSID[3],RADIOINFO->RADIO[0]->BSS[i]->BSSID[4],
								RADIOINFO->RADIO[0]->BSS[i]->BSSID[5],((RADIOINFO->RADIO[0]->BSS[i]->State== 0)?dis:en),whichinterface,RADIOINFO->RADIO[0]->BSS[i]->bss_max_allowed_sta_num,
								wds);
					}
					
					dcli_radio_free_fun(WID_DBUS_CONF_METHOD_SHOWRADIO,RADIOINFO);
				}else if (ret == RADIO_ID_NOT_EXIST)
					vty_out(vty,"<error> radio id does not exist\n");
				else if(ret == RADIO_ID_LARGE_THAN_MAX)		
					vty_out(vty,"<error> radio id should be 1 to %d\n",G_RADIO_NUM);
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
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}

	local_hansi_parameter:
				RADIOINFO = dcli_radio_show_api_group_one(
					profile,
					radio_id,/*"show radio RADIOID"*/
					&localid,
					&ret,
					0,
					dcli_dbus_connection,
					WID_DBUS_CONF_METHOD_SHOWRADIO
					);	
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"---------------------------------------------------------------------------\n");
				if(ret == 0){	
					vty_out(vty,"RadioID:	%d\nWTPID:		%d\nRadio_local_ID:	%d\nAdstate:	%s\nOpstate:	%s\nFragThreshold:  %d\nBeaconPeriod:   %d\nShortPreamble:  %d\nDTIMPeriod:     %d\nRTSThreshold:   %d\nShortRetry:     %d\nLongRetry:      %d\n",\
						radio_id,RADIOINFO->RADIO[0]->WTPID,RADIOINFO->RADIO[0]->Radio_L_ID,((RADIOINFO->RADIO[0]->AdStat== 2)?dis:en),((RADIOINFO->RADIO[0]->OpStat== 2)?dis:en),RADIOINFO->RADIO[0]->FragThreshold,RADIOINFO->RADIO[0]->BeaconPeriod,RADIOINFO->RADIO[0]->IsShortPreamble,RADIOINFO->RADIO[0]->DTIMPeriod,RADIOINFO->RADIO[0]->rtsthreshold,RADIOINFO->RADIO[0]->ShortRetry,RADIOINFO->RADIO[0]->LongRetry);			
					if((RADIOINFO->RADIO[0]->Radio_TXP == 0)||((RADIOINFO->RADIO[0]->Radio_TXP == 100)))
					{
						vty_out(vty,"txpower:	auto\n");
					}
					else 
					{
						vty_out(vty,"txpower:	%d\n",RADIOINFO->RADIO[0]->Radio_TXP);
					}
					
					if (RADIOINFO->RADIO[0]->txpowerautostate == 0)
					{
						vty_out(vty,"txpower state:	auto\n");
					}
					else if(RADIOINFO->RADIO[0]->txpowerautostate == 1)
					{
						vty_out(vty,"txpower state:	manual\n");
					}
					vty_out(vty,"txpowerstep:  %d\n",RADIOINFO->RADIO[0]->txpowerstep);   //fengwenchao add 20110329
					vty_out(vty,"uptime:         %d\n",RADIOINFO->RADIO[0]->upcount);
					vty_out(vty,"downtime:       %d\n",RADIOINFO->RADIO[0]->downcount);
					
					if (RADIOINFO->RADIO[0]->Radio_Chan == 0)
					{
						vty_out(vty,"Channel:	auto\n");
					}
					else 
					{
						vty_out(vty,"Channel:	%d\n",RADIOINFO->RADIO[0]->Radio_Chan);
					}
					if(RADIOINFO->RADIO[0]->auto_channel_cont ==0){
						vty_out(vty,"channel state : auto\n");
					}
					else {
						vty_out(vty,"channel state : manual\n");
					}
					vty_out(vty,"SupportRateCount:	%d\n",RADIOINFO->RADIO[0]->Support_Rate_Count);
					
					if (RADIOINFO->RADIO[0]->Support_Rate_Count != 0)
					{
						vty_out(vty,"Support Rate:   ");
							for (i=0;i<(RADIOINFO->RADIO[0]->Support_Rate_Count);i++)
							{	
								
									vty_out(vty,"%0.1f ",(*(RADIOINFO->RADIO[0]->RadioRate[i])) /10.0);
						}
						vty_out(vty,"M/bps\n");
					}
					
			#if TEST_SWITCH_WAY
					
					vty_out(vty,"radio apply wlan id: ");
						for (i = 0; i < RADIOINFO->wlan_num; i++)
						{	
								vty_out(vty,"%d  ",(RADIOINFO->RADIO[0]->WlanId[i]));
					}
					if(i == 0)
					{
						vty_out(vty,"NONE\n");
					}
					else
					{
						vty_out(vty,"\n");
					}	
					
			#endif
					
					vty_out(vty,"Radio Type:	");
					/*fengwenchao modify begin  20111109 for GM*/
					int flag_an = 0;int flag_gn = 0;
					if(RADIOINFO->RADIO[0]->Radio_Type != 0){				
						vty_out(vty,"11");			

						if(((RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11A) > 0)
							&&((RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11N) > 0)
							&&((RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11AN) > 0)
							&&(!(RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11B))
							&&(!(RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11G)))
						{
							vty_out(vty,"a/an");
						}				
						else if(((RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11GN) > 0)
							&&((RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11N)> 0)
							&&((RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11G)>0)
							&&(!(RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11B))
							&&(!(RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11A)))
						{	
							vty_out(vty,"g/gn");
						}	
						else
						{
							if((RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11A) > 0)
								vty_out(vty,"a");
							if((RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11B) > 0)
								vty_out(vty,"b");
							if((RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11G) > 0)
								vty_out(vty,"g");
							if((RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11N) > 0)
								vty_out(vty,"n");	
						}
					}
					/*fengwenchao modify  end*/
					vty_out(vty,"\n");
					if((RADIOINFO->RADIO[0]->Radio_Type&IEEE80211_11N)>0){
					if(RADIOINFO->RADIO[0]->channel_offset == 0)
						vty_out(vty,"Channel offset:	%s\n","none");
					if(RADIOINFO->RADIO[0]->channel_offset == -1)
						vty_out(vty,"Channel offset:	%s\n","down");
					if(RADIOINFO->RADIO[0]->channel_offset == 1)
						vty_out(vty,"Channel offset:	%s\n","up");

		            vty_out(vty,"Chainmask Number:  %d\n",RADIOINFO->RADIO[0]->chainmask_num);
					vty_out(vty,"Tx_chainmask:   ");
					if((RADIOINFO->RADIO[0]->tx_chainmask_state_value & 0x4)>0)
						vty_out(vty,"1:");
					else
						vty_out(vty,"0:");
					if((RADIOINFO->RADIO[0]->tx_chainmask_state_value & 0x2)>0)
						vty_out(vty,"1:");
					else
						vty_out(vty,"0:");
					if((RADIOINFO->RADIO[0]->tx_chainmask_state_value & 0x1)>0)
						vty_out(vty,"1\n");
					else
						vty_out(vty,"0\n");

				    /* zhangshu add for show Rx_chainmask, 2010-10-09 */
				    vty_out(vty,"Rx_chainmask:   ");
					if((RADIOINFO->RADIO[0]->rx_chainmask_state_value & 0x4)>0)
						vty_out(vty,"1:");
					else
						vty_out(vty,"0:");
					if((RADIOINFO->RADIO[0]->rx_chainmask_state_value & 0x2)>0)
						vty_out(vty,"1:");
					else
						vty_out(vty,"0:");
					if((RADIOINFO->RADIO[0]->rx_chainmask_state_value & 0x1)>0)
						vty_out(vty,"1\n");
					else
						vty_out(vty,"0\n");

					vty_out(vty,"ampdu state:	%s\n",(RADIOINFO->RADIO[0]->Ampdu.Able == 1)?"enable":"disable");
					vty_out(vty,"ampdu limit:	%u\n",RADIOINFO->RADIO[0]->Ampdu.AmpduLimit);

					/* zhangshu add for ampdu & amsdu show, 2010-10-09 */
					vty_out(vty,"ampdu subframe: %d\n",RADIOINFO->RADIO[0]->Ampdu.subframe);
					vty_out(vty,"amsdu state:	%s\n",(RADIOINFO->RADIO[0]->Amsdu.Able == 1)?"enable":"disable");
					vty_out(vty,"amsdu limit:	%u\n",RADIOINFO->RADIO[0]->Amsdu.AmsduLimit);
					vty_out(vty,"amsdu subframe: %d\n",RADIOINFO->RADIO[0]->Amsdu.subframe);
					vty_out(vty,"pureN Mixed:	%s\n",(RADIOINFO->RADIO[0]->MixedGreenfield.Mixed_Greenfield == 1)?"pureN":"Mixed");

					vty_out(vty,"guard interval:	%s\n",(RADIOINFO->RADIO[0]->guardinterval == 1)?"400ns":"800ns");

					if(RADIOINFO->RADIO[0]->cwmode == 0){
						vty_out(vty,"cwmode:	ht20\n");
					}
					else if(RADIOINFO->RADIO[0]->cwmode == 1){
						vty_out(vty,"cwmode:	ht20/40\n");
					}
					else if(RADIOINFO->RADIO[0]->cwmode == 2){
						vty_out(vty,"cwmode:	ht40\n");
						}
					
					vty_out(vty,"mcs count %d\n ",RADIOINFO->RADIO[0]->mcs_count);//qiuchen change
					vty_out(vty,"mcs: ");//qiuchen change
					for(i=0;i<RADIOINFO->RADIO[0]->mcs_count;i++){
						vty_out(vty,"%d, ",RADIOINFO->RADIO[0]->mcs_list[i]);
					}
					vty_out(vty,"\n");
					
					}
					/*wcl add for OSDEVTDPB-31*/
					switch(RADIOINFO->RADIO[0]->Radio_country_code){
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
							vty_out(vty,"country-code : %d\n",RADIOINFO->RADIO[0]->Radio_country_code);
								break;
					}
					/*end*/

					vty_out(vty,"radio %d-%d BSS summary\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM);
					vty_out(vty,"-----------------------------------------------------------------------\n");
					vty_out(vty,"%-6s	%-8s %-17s  %-9s	%-9s %-9s %-8s\n","WLANID","Keyindex","BSSID","BSS State","BSSPolicy","MaxStaNum","WDS_Mode");
					for (i = 0; i < RADIOINFO->bss_num; i++) {
						memset(whichinterface,0,RADIO_IF_NAME_LEN);
						CheckWIDIfPolicy(whichinterface,RADIOINFO->RADIO[0]->BSS[i]->BSS_IF_POLICY);/*sz20080825*/
						memset(wds,0,20);
						if(RADIOINFO->RADIO[0]->BSS[i]->WDSStat == 0){
							strcpy(wds,"disable");
						}else if(RADIOINFO->RADIO[0]->BSS[i]->WDSStat == 1){
							strcpy(wds,"ANY");
						}else if(RADIOINFO->RADIO[0]->BSS[i]->WDSStat == 2){
							strcpy(wds,"SOME");
						}
						vty_out(vty,"%-6d	%-8d %02X:%02X:%02X:%02X:%02X:%02X  %-9s	%-9s %-9d %-8s\n",RADIOINFO->RADIO[0]->BSS[i]->WlanID,RADIOINFO->RADIO[0]->BSS[i]->keyindex,RADIOINFO->RADIO[0]->BSS[i]->BSSID[0],RADIOINFO->RADIO[0]->BSS[i]->BSSID[1],RADIOINFO->RADIO[0]->BSS[i]->BSSID[2],RADIOINFO->RADIO[0]->BSS[i]->BSSID[3],RADIOINFO->RADIO[0]->BSS[i]->BSSID[4],
								RADIOINFO->RADIO[0]->BSS[i]->BSSID[5],((RADIOINFO->RADIO[0]->BSS[i]->State== 0)?dis:en),whichinterface,RADIOINFO->RADIO[0]->BSS[i]->bss_max_allowed_sta_num,
								wds);
					}
					
					dcli_radio_free_fun(WID_DBUS_CONF_METHOD_SHOWRADIO,RADIOINFO);
				}else if (ret == RADIO_ID_NOT_EXIST)
					vty_out(vty,"<error> radio id does not exist\n");
				else if(ret == RADIO_ID_LARGE_THAN_MAX)		
					vty_out(vty,"<error> radio id should be 1 to %d\n",G_RADIO_NUM);
				vty_out(vty,"==============================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}
	}

	return CMD_SUCCESS;
}

DEFUN(config_radio_cmd_func,
	  config_radio_cmd,
	  "config radio RADIOID",
	  CONFIG_STR
	  "Radio information\n"
	  "Radio id format is wtpid-localradioid 1-0 or 5-1\n"
	 )
{	int ret;
	unsigned int radio_id;

	int wtpid = 0;
	int l_radioid = 4;

	DBusMessage *query, *reply;
	DBusError err;

	ret = parse_radio_id((char*)argv[0],&wtpid,&l_radioid);

	if (ret != WID_DBUS_SUCCESS) 
	{
		ret = parse_int_ID((char*)argv[0], &radio_id);
		
		if(ret != WID_DBUS_SUCCESS){

			vty_out(vty,"<error> error parameter format correct format is wtpid-radioid [1-0][3-1]\n");
			
			return CMD_WARNING;
		}
	}
	else
	{	
		radio_id = wtpid*L_RADIO_NUM + l_radioid;
	}
	//ret = parse_int_ID((char*)argv[0], &radio_id);

	
	//if(ret != WID_DBUS_SUCCESS){
	//	vty_out(vty,"<error> unknown id format\n");
	//	return CMD_SUCCESS;
	//}
	
	if(radio_id > G_RADIO_NUM || radio_id == 0){
		vty_out(vty,"<error> radio id should be 1-%d\n",G_RADIO_NUM);
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
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_RADIO);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_RADIO);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_DBUS_ERR_RETRY;
	}
	
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&ret,
					DBUS_TYPE_INVALID)) {
		if(ret == 0){
			if(vty->node == CONFIG_NODE){
				vty->node = RADIO_NODE;
				vty->index = (void *)radio_id;
			}else if(vty->node == HANSI_NODE){			
				vty->node = HANSI_RADIO_NODE;
				vty->index_sub = (void *)radio_id;
			}else if(vty->node == LOCAL_HANSI_NODE){			
				vty->node = LOCAL_HANSI_RADIO_NODE;
				vty->index_sub = (void *)radio_id;
			}
		}else if (ret == RADIO_ID_NOT_EXIST){
			vty_out(vty,"<error> radio id does not exist\n");		
			dbus_message_unref(reply);
			return CMD_WARNING;
		}
		else if(ret == RADIO_ID_LARGE_THAN_MAX){		
			vty_out(vty,"<error> radio id should be 1 to %d\n",G_RADIO_NUM);
			dbus_message_unref(reply);
			return CMD_WARNING;
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

#if _GROUP_POLICY
/*sz1121 add channel 0*/
DEFUN(set_radio_channel_cmd_func,
	  set_radio_channel_cmd,
	  /*"channel <1-11>",*/
	  "channel CHANNEL",
	  "set radio channel\n"
	  "Radio channel value\n"
	 )
{
	int ret = 0;
	int ret1 = 0;
	int ret2 = CHANNEL_CWMODE_SUCCESS;
	unsigned short cwmode = 0;
	char channel_offset = 0;
	char mode[3][8] = {"ht20","ht20/40","ht40"};

	int i = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned char channel = 0;	
	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		

	int check_channel = 0;
	if (!strcmp(argv[0],"auto"))
	{

    	channel = 0;
	}
	else 
	{

		ret = parse_char_ID((char *)argv[0],&channel);
		if (ret != WID_DBUS_SUCCESS)
		{	
           if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
			vty_out(vty,"<error> input parameter %s error\n",argv[0]);
			}
			return CMD_SUCCESS;
		}
		if ((channel <= 0)||(channel > 14)) 
		{
			if (!((channel == 36)||(channel == 40)||(channel == 44)||(channel == 46)||(channel == 48)\
				||(channel == 52)||(channel == 56)||(channel == 60)||(channel == 64)||(channel == 100)\
				||(channel == 104)||(channel == 108)||(channel == 112)||(channel == 116)||(channel == 120)\
				||(channel == 124)||(channel == 128)||(channel == 132)||(channel == 136)||(channel == 140)\
				||(channel == 149)||(channel == 153)||(channel == 157)||(channel == 161)||(channel == 165)\
				||(channel == 184)||(channel == 188)||(channel == 192)||(channel == 196)\
				)) /*wcl modify for AUTELAN-2765*/
			{
				vty_out(vty,"<error> input parameter %s error\n",argv[0]);
				vty_out(vty,"11a receive channel list is:  36 ..;149 153 157 161\n");
				return CMD_SUCCESS;
			}
		}
	}
	
	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		id = (unsigned)vty->index_sub;
		localid = vty->local;
        slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
        type= 1;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = set_radio_channel_cmd_channel(localid,index,dcli_dbus_connection,type,id,channel,&count,&ret,&ret1,&ret2,&cwmode,&channel_offset);
		

	if(type == 0)
		{
			if(ret == 0)
				{
					switch(ret1)
					{		
						case COUNTRY_CHINA_CN : vty_out(vty,"<error> channel %s is invalid in CHINA\n",argv[0]);
												break;
						case COUNTRY_EUROPE_EU : vty_out(vty,"<error> channel %s is invalid in EUROPE\n",argv[0]);
												break;
		        		case COUNTRY_USA_US : vty_out(vty,"<error> channel %s is invalid in USA\n",argv[0]);
												break;
						case COUNTRY_JAPAN_JP : vty_out(vty,"<error> channel %s is invalid in JAPAN\n",argv[0]);
												break;
						case COUNTRY_FRANCE_FR : vty_out(vty,"<error> channel %s is invalid in FRANCE\n",argv[0]);
												break;
						case COUNTRY_SPAIN_ES : vty_out(vty,"<error> channel %s is invalid in SPAIN\n",argv[0]);
												break;
						case COUNTRY_CODE_SUCCESS : break;
						default : break;
					}
				if(ret2 != CHANNEL_CWMODE_SUCCESS)
					{
						vty_out(vty,"<error> cwmode is %s,and channel offset is %s\n",(mode[cwmode]),((channel_offset == -1)?"down":"up"));					
					}
				else if(ret2 == CHANNEL_CWMODE_SUCCESS)
					{
						if (ret1 == COUNTRY_CODE_SUCCESS)
							{
								if (channel == 0)
									{
										vty_out(vty,"Radio %d-%d channel was set auto successfully .\n",id/L_RADIO_NUM,id%L_RADIO_NUM);
									}
								else 
									{
										vty_out(vty,"Radio %d-%d channel was set %s successfully .\n",id/L_RADIO_NUM,id%L_RADIO_NUM,argv[0]);
									}
							}
					}
				}
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio id does not exist\n");
			else if(ret == RADIO_IS_DISABLE)
				vty_out(vty,"<error> radio is disable, please enable it first\n");
			else if(ret == WTP_NO_SURPORT_CHANNEL)
				vty_out(vty,"<error> radio type doesn,t support this channel\n");
			else
				vty_out(vty,"<error>  %d\n",ret);			
		}
	else if(type==1)
		{
			if(ret == 0)
				{	
					if(channel == 0)
						vty_out(vty,"group %d Radio channel was set auto successfully .\n",id,argv[0]);
					else
						vty_out(vty,"group %d Radio channel was set %s successfully .\n",id,argv[0]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");		
		}	
	return CMD_SUCCESS;

}

#else
/*sz1121 add channel 0*/
DEFUN(set_radio_channel_cmd_func,
	  set_radio_channel_cmd,
	  /*"channel <1-11>",*/
	  "channel CHANNEL",
	  "set radio channel\n"
	  "Radio channel value\n"
	 )
{
	unsigned char channel;
	unsigned int radio_id; 
	int ret = 0;
	int ret1 = 0;
	int ret2 = CHANNEL_CWMODE_SUCCESS;
	unsigned short cwmode = 0;
	char channel_offset = 0;
	char mode[3][8] = {"ht20","ht20/40","ht40"};
	//char eth[4][5] = {"eth0","eth1","eth2","eth3"};
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;	
	int check_channel = 0;
	if (!strcmp(argv[0],"auto"))
	{
		/*printf("input is auto\n");*/
    	channel = 0;
	}
	else 
	{
		/*printf("input is not auto\n");
		//channel = atoi(argv[0]);*/
		ret = parse_char_ID((char *)argv[0],&channel);
		if (ret != WID_DBUS_SUCCESS)
		{	
           if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
			vty_out(vty,"<error> input parameter %s error\n",argv[0]);
			}
			return CMD_SUCCESS;
		}
		if ((channel <= 0)||(channel > 14)) 
		{
			if (!((channel == 36)||(channel == 40)||(channel == 44)||(channel == 46)||(channel == 48)\
				||(channel == 52)||(channel == 56)||(channel == 60)||(channel == 64)||(channel == 100)\
				||(channel == 104)||(channel == 108)||(channel == 112)||(channel == 116)||(channel == 120)\
				||(channel == 124)||(channel == 128)||(channel == 132)||(channel == 136)||(channel == 140)\
				||(channel == 149)||(channel == 153)||(channel == 157)||(channel == 161)||(channel == 165)\
				||(channel == 184)||(channel == 188)||(channel == 192)||(channel == 196)\
				)) /*wcl modify for AUTELAN-2765*/
			{
				vty_out(vty,"<error> input parameter %s error\n",argv[0]);
				vty_out(vty,"11a receive channel list is:  36 ..;149 153 157 161\n");
				return CMD_SUCCESS;
			}
		}
	}
	//radio_id = (int)vty->index;
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_CHAN);
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_SET_CHAN);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_BYTE,&channel,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
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
	dbus_message_iter_get_basic(&iter,&ret1);
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&ret2);
	if(ret2 != CHANNEL_CWMODE_SUCCESS){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&cwmode);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&channel_offset);
	}
		if(ret == 0)
		{
			switch(ret1)
			{
				case COUNTRY_CHINA_CN : vty_out(vty,"<error> channel %s is invalid in CHINA\n",argv[0]);
										break;
				case COUNTRY_EUROPE_EU : vty_out(vty,"<error> channel %s is invalid in EUROPE\n",argv[0]);
										break;
		        case COUNTRY_USA_US : vty_out(vty,"<error> channel %s is invalid in USA\n",argv[0]);
										break;
				case COUNTRY_JAPAN_JP : vty_out(vty,"<error> channel %s is invalid in JAPAN\n",argv[0]);
										break;
				case COUNTRY_FRANCE_FR : vty_out(vty,"<error> channel %s is invalid in FRANCE\n",argv[0]);
										break;
				case COUNTRY_SPAIN_ES : vty_out(vty,"<error> channel %s is invalid in SPAIN\n",argv[0]);
										break;
				case COUNTRY_CODE_SUCCESS : break;
				default : break;
			}
			if(ret2 != CHANNEL_CWMODE_SUCCESS)
			{
				vty_out(vty,"<error> cwmode is %s,and channel offset is %s\n",(mode[cwmode]),((channel_offset == -1)?"down":"up"));
								
			}else if(ret2 == CHANNEL_CWMODE_SUCCESS){
				if (ret1 == COUNTRY_CODE_SUCCESS)
				{
					if (channel == 0)
					{
						vty_out(vty,"Radio %d-%d channel was set auto successfully .\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM);
					}
					else 
					{
						vty_out(vty,"Radio %d-%d channel was set %s successfully .\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0]);
					}
				}
			}
		}
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio id does not exist\n");
		else if(ret == RADIO_IS_DISABLE)
			vty_out(vty,"<error> radio is disable, please enable it first\n");
		else if(ret == WTP_NO_SURPORT_CHANNEL)
			vty_out(vty,"<error> radio type doesn,t support this channel\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;

}
#endif
#if _GROUP_POLICY
DEFUN(set_radio_txpower_cmd_func,
	  set_radio_txpower_cmd,
	  "txpower TXPOWER",
	  "set radio txpower\n"
	  "Radio txpower value\n"
	 )
{
	u_int16_t	txp;
	unsigned int txpwer = 0;
	int ret1 = COUNTRY_CODE_SUCCESS;
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	
	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		

	if (!strcmp(argv[0],"auto"))
	{
    	txp = 100;
		
	}else{
	ret = parse_short_ID((char *)argv[0],&txp);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
       if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
       }
	   else{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
	   }
		return CMD_SUCCESS;
	}
	if ((txp <= 0)||(txp > 27)) 
	{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}
	}
	

	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
        type= 1;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = set_radio_txpower_cmd_txpower(localid,index,dcli_dbus_connection,type,id,txp,&count,&ret,&ret1);	

	if(type==0)
		{
			if(ret == 0)
				{
					if (ret1 == COUNTRY_CODE_SUCCESS)
						{
							vty_out(vty,"Radio %d-%d txpower was set %s successfully .\n",id/L_RADIO_NUM,id%L_RADIO_NUM,argv[0]);
						}
					else if (ret1 == COUNTRY_CODE_ERROR)
						{
							vty_out(vty,"<error> txpower %s conflict with country-code\n",argv[0]);
						}
				}
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio id does not exist\n");
			else if(ret == RADIO_IS_DISABLE)
				vty_out(vty,"<error> radio is disable, please enable it first\n");
			else if(ret == RADIO_MODE_IS_11N)
					vty_out(vty,"<error> radio mode is 11n,not allow to set txpower\n");
			else if(ret == TXPOWER_OVER_TW)
				vty_out(vty,"<error> this radio max txpower is 20\n");
			else if(ret == TXPOWER_OVER_TW_THREE)
				vty_out(vty,"<error> this radio max txpower is 27\n");   //fengwenchao modify 20110329
			else
				vty_out(vty,"<error>  %d\n",ret);
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d Radio txpower was set %s successfully .\n",id,argv[0]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");		
		}	
	return CMD_SUCCESS;

}

#else
DEFUN(set_radio_txpower_cmd_func,
	  set_radio_txpower_cmd,
	  "txpower TXPOWER",
	  "set radio txpower\n"
	  "Radio txpower value\n"
	 )
{
	unsigned int radio_id; 
	u_int16_t	txp;
	int ret=0;
	int ret1 = COUNTRY_CODE_SUCCESS;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;	
    /*txp = atoi(argv[0]);*/
	if (!strcmp(argv[0],"auto"))
	{
    	txp = 100;
		
	}else{
	ret = parse_short_ID((char *)argv[0],&txp);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
       if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
       }
	   else{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
	   }
		return CMD_SUCCESS;
	}
	if ((txp <= 0)||(txp > 27)) 
	{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}
	}
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_TXP);
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_SET_TXP);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_UINT16,&txp,
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
	dbus_message_iter_get_basic(&iter,&ret1);

		if(ret == 0)
		{
			if (ret1 == COUNTRY_CODE_SUCCESS)
			{
				vty_out(vty,"Radio %d-%d txpower was set %s successfully .\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0]);
			}
			else if (ret1 == COUNTRY_CODE_ERROR)
			{
				vty_out(vty,"<error> txpower %s conflict with country-code\n",argv[0]);
			}
		}
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio id does not exist\n");
		else if(ret == RADIO_IS_DISABLE)
			vty_out(vty,"<error> radio is disable, please enable it first\n");
		else if(ret == RADIO_MODE_IS_11N)
				vty_out(vty,"<error> radio mode is 11n,not allow to set txpower\n");
		else if(ret == TXPOWER_OVER_TW)
			vty_out(vty,"<error> this radio max txpower is 20\n");
		else if(ret == TXPOWER_OVER_TW_THREE)
			vty_out(vty,"<error> this radio max txpower is 27\n");   //fengwenchao modify 20110329
		
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;

}
#endif
/*added by weay 20080714
//sz1121 add rate 0*/
/*DEFUN(set_radio_rate_cmd_func,
	  set_radio_rate_cmd,
	  //"rate <10-540>",
	  "rate RATE",
	  "set radio rate\n"
	  "Radio rate value(10 20 55 60 90 110 120 180 240 360 480 540 auto)\n"
	 )
{
	unsigned int radio_id,mode; 
	unsigned short rate;
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;	
    //rate = atoi(argv[0]);
	if (!strcmp(argv[0],"auto"))
	{
    	rate = 0;
	}
	else 
	{
		ret = parse_short_ID((char *)argv[0],&rate);
		if (ret != WID_DBUS_SUCCESS)
		{
			return CMD_SUCCESS;
		}
		if((rate < 10)||(rate > 540))
		{
			vty_out(vty,"<error> input parameter should be <10-540>\n");
			vty_out(vty,"You should choose rate list: 10 20 55 60 90 110 120 180 240 360 480 540 \n");
			return CMD_SUCCESS;
		}
	}
	
	radio_id = (int)vty->index;
	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_SET_RATE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_UINT16,&rate,
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
	dbus_message_iter_get_basic(&iter,&mode);
	
		if(ret == 0){
			if (rate == 0){
				vty_out(vty,"Radio %d rate was set auto successfully .\n",radio_id);
				}
			else {
				vty_out(vty,"Radio %d rate was set %s successfully .\n",radio_id,argv[0]);
			}
			}
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio id does not exist\n");
		else if(ret == WTP_NO_SURPORT_Rate)
		{
			if (mode == 1)
			{
				vty_out(vty,"<error> wtp mode is 11b,does not support rate %s\n",argv[0]);
				vty_out(vty,"mode 11b support rate list:auto 10 20 55 110\n");
			}
			else if (mode == 4)
			{
				vty_out(vty,"<error> wtp mode is 11g,does not support rate %s\n",argv[0]);
				vty_out(vty,"mode 11g support rate list:auto 10 20 55 60 90 110 120 180 240 360 480 540\n");
			}
			else if (mode == 5)
			{
				vty_out(vty,"<error> wtp mode is 11b/g,does not support rate %s\n",argv[0]);
				vty_out(vty,"mode 11b/g support rate list:auto 10 20 55 110\n");
			}
			else
			{
				vty_out(vty,"<error> wtp radio does not support this rate,please check first\n");
			}
		}
		else if(ret == RADIO_IS_DISABLE)
			vty_out(vty,"<error> radio is disable, please enable it first\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;

}*/
#if _GROUP_POLICY
DEFUN(set_radio_txpowerof_cmd_func,
	  set_radio_txpowerof_cmd,
	  "txpoweroffset VALUE",/*wuwl add 0928 */    //fengwenchao change <0-27> to VALUE  20110504
	  "set radio txpower offset\n"
	  "Radio txpowerof value  <0-27>\n"          //fengwenchao modify 20110504
	 )
 {
	u_int16_t	txpof;
	unsigned int txpwer = 0;

	int ret1 = COUNTRY_CODE_SUCCESS;

	int i = 0;
	int ret = 0;
	int count = 0;
	//int country_fail_count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	
	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		

	ret = parse_int_ID((char *)argv[0],&txpwer);
	txpof = (u_int16_t)txpwer;
	vty_out(vty,"txpof = %d \n",txpof);
	vty_out(vty,"txpwer = %d \n",txpwer);
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}
	if ((txpof > 27)) 
	{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}
	//radio_id = (int)vty->index;
	

	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = set_radio_txpowerof_cmd_txpoweroffset(localid,index,dcli_dbus_connection,type,id,txpof,&count,&ret,&ret1);	

	if(type==0)
		{
			if(ret == 0)
				{
					if (ret1 == COUNTRY_CODE_SUCCESS)
						{
							vty_out(vty,"Radio %d-%d txpower offset was set %s successfully .\n",id/L_RADIO_NUM,id%L_RADIO_NUM,argv[0]);
						}
					else if (ret1 == COUNTRY_CODE_ERROR)
						{
							vty_out(vty,"<error> txpower %s conflict with country-code\n",argv[0]);
						}
				}
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio id does not exist\n");
			else if(ret == RADIO_IS_DISABLE)
				vty_out(vty,"<error> radio is disable, please enable it first\n");
			else if(ret == RADIO_MODE_IS_11N)
					vty_out(vty,"<error> radio mode is 11n,not allow to set txpower\n");
			else if(ret == TXPOWER_OVER_TW)
				vty_out(vty,"<error> this radio max txpower is 20\n");
			else if(ret == TXPOWER_OVER_TW_THREE)
				vty_out(vty,"<error> this radio max txpower is 27\n");  //fengwenchao modify 20110329
			else if(ret == WTP_NOT_IN_RUN_STATE)
				vty_out(vty,"<error> this wtp is not in run state.\n");		
			else if(ret == RADIO_NO_BINDING_WLAN)
				vty_out(vty,"<error> this radio is not binding wlan,binding wlan first.\n");	
			else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
				vty_out(vty,"<error> radio doesn't bind wlan\n");
			else if(ret == TXPOWEROFF_LARGER_THAN_MAX)
				vty_out(vty,"txpoweroffset is larger than max txpower!Please checkout txpowerstep!!\n");
			else
				vty_out(vty,"<error>  %d\n",ret);
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d Radio txpower offset was set %s successfully .\n",id,argv[0]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}	
	return CMD_SUCCESS;

}

#else
DEFUN(set_radio_txpowerof_cmd_func,
	  set_radio_txpowerof_cmd,
	  "txpoweroffset VALUE",/*wuwl add 0928 */       //fengwenchao change <0-27> to VALUE  20110504
	  "set radio txpower offset\n"
	  "Radio txpowerof value <0-27>\n"         //fengwenchao modify 20110504
	 )
	 {
	unsigned int radio_id; 
	u_int16_t	txpof;
	unsigned int txpwer = 0;
	int ret=0;
	int ret1 = COUNTRY_CODE_SUCCESS;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;	
    /*txp = atoi(argv[0]);*/
	ret = parse_int_ID((char *)argv[0],&txpwer);
	txpof = (u_int16_t)txpwer;
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}
	if ((txpof > 27)) 
	{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}
	//radio_id = (int)vty->index;
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_TXPOF);
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_SET_TXPOF);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_UINT16,&txpof,
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
	dbus_message_iter_get_basic(&iter,&ret1);

		if(ret == 0)
		{
			if (ret1 == COUNTRY_CODE_SUCCESS)
			{
				vty_out(vty,"Radio %d-%d txpower offset was set %s successfully .\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0]);
			}
			else if (ret1 == COUNTRY_CODE_ERROR)
			{
				vty_out(vty,"<error> txpower %s conflict with country-code\n",argv[0]);
			}
		}
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio id does not exist\n");
		else if(ret == RADIO_IS_DISABLE)
			vty_out(vty,"<error> radio is disable, please enable it first\n");
		else if(ret == RADIO_MODE_IS_11N)
				vty_out(vty,"<error> radio mode is 11n,not allow to set txpower\n");
		else if(ret == TXPOWER_OVER_TW)
			vty_out(vty,"<error> this radio max txpower is 20\n");
		else if(ret == TXPOWER_OVER_TW_THREE)
			vty_out(vty,"<error> this radio max txpower is 27\n");   //fengwenchao modify 20110329
		else if(ret == WTP_NOT_IN_RUN_STATE)
			vty_out(vty,"<error> this wtp is not in run state.\n");		
		else if(ret == RADIO_NO_BINDING_WLAN)
			vty_out(vty,"<error> this radio is not binding wlan,binding wlan first.\n");	
		else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
			vty_out(vty,"<error> radio doesn't bind wlan\n");
		else if(ret == TXPOWEROFF_LARGER_THAN_MAX)
			vty_out(vty,"txpoweroffset is larger than max txpower!Please checkout txpowerstep!!\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;

}
#endif

#if _GROUP_POLICY
DEFUN(set_radio_ratelist_cmd_func,
	  set_radio_ratelist_cmd,
	  "set support ratelist RATELIST",
	  "set radio rate\n"
	  "Radio rate value(10 20 55 60 90 110 120 180 240 360 480 540)\n"
	 )
{

		int n=0;
		int num=0;
		int i = 0;
		int list[RADIO_RATE_LIST_LEN];
		int list1[RADIO_RATE_LIST_LEN];
		int ret = 0;
		int ret1 = 0;
		int count = 0;
		int index = 0;
		int localid = 1;
        int slot_id = HostSlotId;
		int mode = 0;
		int rate = 0;
		unsigned int id = 0;
		unsigned int type = 0;
		
		struct RadioList *RadioList_Head = NULL;
		struct RadioList *Radio_Show_Node = NULL;
			
		 	

	
		/*parse the rate list*/
		ret = parse_rate_list((char *)argv[0],&n,list);
	
		if (ret != WID_DBUS_SUCCESS)
		{
			vty_out(vty,"<error> input parameter should be <10-540>\n");
			vty_out(vty,"You should choose rate list: 10 20 55 60 90 110 120 180 240 360 480 540 \n");
			return CMD_SUCCESS;
		}
			
		/*remove the repeat rate,process the order*/
		num = process_rate_list(list,n);/*the num in the rate list*/
			
		if(vty->node == RADIO_NODE){
			index = 0;			
			id = (unsigned)vty->index;
		}else if(vty->node == HANSI_RADIO_NODE){
			index = vty->index; 		
			localid = vty->local;
            slot_id = vty->slotindex;
			id = (unsigned)vty->index_sub;
		}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
            index = vty->index;
            localid = vty->local;
            slot_id = vty->slotindex;
			id = (unsigned)vty->index_sub;
        }else if(vty->node == AP_GROUP_RADIO_NODE){
			index = 0;			
			id = (unsigned)vty->index;
			type = 1;
			vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
		}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
			index = vty->index; 
			localid = vty->local;
            slot_id = vty->slotindex;
			id = (unsigned)vty->index_sub;
			type= 1;
		}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
            index = vty->index;
            localid = vty->local;
            slot_id = vty->slotindex;
            id = (unsigned)vty->index_sub;
			type= 1;
        }
        DBusConnection *dcli_dbus_connection = NULL;
        ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

			
		RadioList_Head = set_radio_ratelist_cmd_set_support_ratelist(localid,index,dcli_dbus_connection,type,id,num,&count,&ret,&ret1,list,&mode);
		
		if(type==0)
			{
				if(ret==0)
					{
						if(ret1 == -1)
							vty_out(vty,"<error> an unexpect error\n");
						else
							{
											
								vty_out(vty,"set radio %d-%d support ratelist successfully\n",id/L_RADIO_NUM,id%L_RADIO_NUM);
								vty_out(vty,"radio support ratenum : %d\n",count);
								vty_out(vty,"radio support ratelist : ");
								for(i=0;i<count;i++)
								{
									vty_out(vty,"%d ",list[i]);
								}
								vty_out(vty," \n");
							}
							
					}
				
				else if(ret == RADIO_ID_NOT_EXIST)
					vty_out(vty,"<error> radio id does not exist\n");
				else if(ret == WTP_NO_SURPORT_Rate)
				{
					if (mode == 1)
					{
						vty_out(vty,"<error> wtp mode is 11b,does not support rate\n");
						vty_out(vty,"mode 11b support rate list:10 20 55 110\n");
					}
					else if (mode == 2)
					{
						vty_out(vty,"<error> wtp mode is 11a,does not support rate\n");
						vty_out(vty,"mode 11a support rate list:60 90 120 180 240 360 480 540\n");
					}
					else if (mode == 4)
					{
						vty_out(vty,"<error> wtp mode is 11g,does not support rate\n");
						vty_out(vty,"mode 11g support rate list:60 90 120 180 240 360 480 540\n");
					}
					else if (mode == 10)
					{
						vty_out(vty,"<error> wtp mode is 11an,does not support rate\n");
						vty_out(vty,"mode 11an support rate list:60 90 120 180 240 360 480 540\n");
					}
					else if (mode == 12)
					{
						vty_out(vty,"<error> wtp mode is 11gn,does not support rate\n");
						vty_out(vty,"mode 11gn support rate list:60 90 120 180 240 360 480 540\n");
					}
					else if (mode == 26)
					{
						vty_out(vty,"<error> wtp mode is 11a/an,does not support rate\n");
						vty_out(vty,"mode 11a/an support rate list:60 90 120 180 240 360 480 540\n");
					}
					else if (mode == 44)
					{
						vty_out(vty,"<error> wtp mode is 11g/gn,does not support rate\n");
						vty_out(vty,"mode 11g/gn support rate list:60 90 120 180 240 360 480 540\n");
					}				
					else if (mode == 5)
					{
						vty_out(vty,"<error> wtp mode is 11b/g,does not support rate\n");
						vty_out(vty,"mode 11b/g support rate list:10 20 55 60 90 110 120 180 240 360 480 540\n");
					}
					else if (mode == 13)
					{
						vty_out(vty,"<error> wtp mode is 11b/g/n,does not support rate\n");
						vty_out(vty,"mode 11b/g/n support rate list:10 20 60 90 110 120 180 240 360 480 540\n");
					}
					else
					{
						vty_out(vty,"<error> wtp radio does not support this rate,please check first\n");
					}
				}
				else if(ret == RADIO_IS_DISABLE)
					vty_out(vty,"<error> radio is disable, please enable it first\n");
				else if(ret == RADIO_SUPPORT_RATE_EMPTY)
					vty_out(vty,"<error> radio list is empty\n");
				else if(ret == RADIO_MODE_IS_11N)
					vty_out(vty,"<error> radio mode is 11n,not allow to set rate\n");
				else if(ret == RADIO_SUPPORT_RATE_NOT_EXIST)/*use in delete support ratelist,maybe later*/
					vty_out(vty,"<error> radio support rate does not exist\n");
				else if((ret == WTP_NO_SURPORT_TYPE)||(ret == RADIO_SUPPORT_RATE_CONFLICT))
					vty_out(vty,"<error> radio type is conflict, please check it first\n");
				else
					vty_out(vty,"<error>  %d\n",ret);
			}
		else if(type==1)
			{
			if(ret == 0)
				{
					vty_out(vty,"group %d set radio support ratelist successfully\n",id);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}	
		
		return CMD_SUCCESS;
}

#else
DEFUN(set_radio_ratelist_cmd_func,
	  set_radio_ratelist_cmd,
	  "set support ratelist RATELIST",
	  "set radio rate\n"
	  "Radio rate value(10 20 55 60 90 110 120 180 240 360 480 540)\n"
	 )
{
	
		int radio_id = 0;
		int mode = 0;
		int ret=0;
		int n=0;
		int num=0;
		int i = 0;
		int list[RADIO_RATE_LIST_LEN];
		int list1[RADIO_RATE_LIST_LEN];
		DBusMessage *query, *reply; 
		DBusMessageIter  iter,iter_array;
		DBusError err;	
		dbus_error_init(&err);
	
		/*parse the rate list*/
		ret = parse_rate_list((char *)argv[0],&n,list);
	
		if (ret != WID_DBUS_SUCCESS)
		{
			vty_out(vty,"<error> input parameter should be <10-540>\n");
			vty_out(vty,"You should choose rate list: 10 20 55 60 90 110 120 180 240 360 480 540 \n");
			return CMD_SUCCESS;
		}
		
	
		/*remove the repeat rate,process the order*/
		num = process_rate_list(list,n);/*the num in the rate list*/
			
		
		/*for (i=0;i<num;i++)
		//{
		//	printf("%d radiorate %d\n",i,list[i]);
		//}*/
	//	radio_id = (int)vty->index;
		
		int index = 0;
		int localid = 1;
        int slot_id = HostSlotId;
		char BUSNAME[PATH_LEN];
		char OBJPATH[PATH_LEN];
		char INTERFACE[PATH_LEN];
		if(vty->node == RADIO_NODE){
			index = 0;			
			radio_id = (int)vty->index;
		}else if(vty->node == HANSI_RADIO_NODE){
			index = vty->index; 
			localid = vty->local;
            slot_id = vty->slotindex;
			radio_id = (int)vty->index_sub;
		}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
            index = vty->index;
            localid = vty->local;
            slot_id = vty->slotindex;
			radio_id = (int)vty->index_sub;
        }
        DBusConnection *dcli_dbus_connection = NULL;
        ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
		ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
		ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_SUPPORT_RATELIST);
			
	/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
							WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_SET_SUPPORT_RATELIST);*/
		
			
		dbus_message_iter_init_append (query, &iter);
	
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&radio_id);
		
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&num);
			
	
	
		for(i = 0; i < num; i++)
		{
			dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&list[i]);
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
		dbus_message_iter_get_basic(&iter,&mode);
		
			if(ret == 0)
			{
				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&num);
				if(num > 20)
				{
					vty_out(vty,"<error> an unexpect error\n");
					return CMD_SUCCESS;
				}
				for(i=0;i<num;i++)
				{
					dbus_message_iter_next(&iter);
					dbus_message_iter_get_basic(&iter,&list1[i]);
				}
				vty_out(vty,"set radio %d-%d support ratelist successfully\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM);
				vty_out(vty,"radio support ratenum : %d\n",num);
				vty_out(vty,"radio support ratelist : ");
				for(i=0;i<num;i++)
				{
					vty_out(vty,"%d ",list1[i]);
				}
				vty_out(vty," \n");
			}
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio id does not exist\n");
			else if(ret == WTP_NO_SURPORT_Rate)
			{
				if (mode == 1)
				{
					vty_out(vty,"<error> wtp mode is 11b,does not support rate\n");
					vty_out(vty,"mode 11b support rate list:10 20 55 110\n");
				}
				else if (mode == 2)
				{
					vty_out(vty,"<error> wtp mode is 11a,does not support rate\n");
					vty_out(vty,"mode 11a support rate list:60 90 120 180 240 360 480 540\n");
				}
				else if (mode == 4)
				{
					vty_out(vty,"<error> wtp mode is 11g,does not support rate\n");
					vty_out(vty,"mode 11g support rate list:60 90 120 180 240 360 480 540\n");
				}
				else if (mode == 10)
				{
					vty_out(vty,"<error> wtp mode is 11an,does not support rate\n");
					vty_out(vty,"mode 11an support rate list:60 90 120 180 240 360 480 540\n");
				}
				else if (mode == 12)
				{
					vty_out(vty,"<error> wtp mode is 11gn,does not support rate\n");
					vty_out(vty,"mode 11gn support rate list:60 90 120 180 240 360 480 540\n");
				}
				else if (mode == 26)
				{
					vty_out(vty,"<error> wtp mode is 11a/an,does not support rate\n");
					vty_out(vty,"mode 11a/an support rate list:60 90 120 180 240 360 480 540\n");
				}
				else if (mode == 44)
				{
					vty_out(vty,"<error> wtp mode is 11g/gn,does not support rate\n");
					vty_out(vty,"mode 11g/gn support rate list:60 90 120 180 240 360 480 540\n");
				}				
				else if (mode == 5)
				{
					vty_out(vty,"<error> wtp mode is 11b/g,does not support rate\n");
					vty_out(vty,"mode 11b/g support rate list:10 20 55 60 90 110 120 180 240 360 480 540\n");
				}
				else if (mode == 13)
				{
					vty_out(vty,"<error> wtp mode is 11b/g/n,does not support rate\n");
					vty_out(vty,"mode 11b/g/n support rate list:10 20 60 90 110 120 180 240 360 480 540\n");
				}

				
				else
				{
					vty_out(vty,"<error> wtp radio does not support this rate,please check first\n");
				}
			}
			else if(ret == RADIO_IS_DISABLE)
				vty_out(vty,"<error> radio is disable, please enable it first\n");
			else if(ret == RADIO_SUPPORT_RATE_EMPTY)
				vty_out(vty,"<error> radio list is empty\n");
			else if(ret == RADIO_MODE_IS_11N)
				vty_out(vty,"<error> radio mode is 11n,not allow to set rate\n");
			else if(ret == RADIO_SUPPORT_RATE_NOT_EXIST)/*use in delete support ratelist,maybe later*/
				vty_out(vty,"<error> radio support rate does not exist\n");
			else if((ret == WTP_NO_SURPORT_TYPE)||(ret == RADIO_SUPPORT_RATE_CONFLICT))
				vty_out(vty,"<error> radio type is conflict, please check it first\n");
			else
				vty_out(vty,"<error>  %d\n",ret);
		

		dbus_message_unref(reply);
		
		return CMD_SUCCESS;
	

}
#endif
#if _GROUP_POLICY
DEFUN(set_radio_max_rate_cmd_func,
	  set_radio_max_rate_cmd,
	  "set max rate RATE",
	  "set radio rate\n"
	  "Radio rate value(10 20 55 60 90 110 120 180 240 360 480 540)\n"
	 )
{
		int num=0;
		int list1[RADIO_RATE_LIST_LEN];
		//int ret1 = COUNTRY_CODE_SUCCESS;
	
		int i = 0;
		int ret = 0;
		int ret1 = 0;
		int count = 0;
		int index = 0;
		int localid = 1;
        int slot_id = HostSlotId;
		int mode = 0;
		int rate = 0;
		unsigned int id = 0;
		unsigned int type = 0;
		
		struct RadioList *RadioList_Head = NULL;
		struct RadioList *Radio_Show_Node = NULL;
			
		 					
		/*parse the rate */
		ret = parse_int_ID((char *)argv[0],&rate);
		if (ret != WID_DBUS_SUCCESS)
		{
			vty_out(vty,"<error> input parameter should be <10-540>\n");
			vty_out(vty,"You should choose rate list: 10 20 55 60 90 110 120 180 240 360 480 540 \n");
			return CMD_SUCCESS;
		}
		
		
		if(vty->node == RADIO_NODE){
			index = 0;			
			id = (int)vty->index;
		}else if(vty->node == HANSI_RADIO_NODE){
			index = vty->index; 		
			localid = vty->local;
            slot_id = vty->slotindex;
			id = (int)vty->index_sub;
		}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
            index = vty->index;
            localid = vty->local;
            slot_id = vty->slotindex;
			id = (int)vty->index_sub;
        }else if(vty->node == AP_GROUP_RADIO_NODE){
			index = 0;			
			id = (unsigned)vty->index;
			type = 1;
			vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
		}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
			index = vty->index; 
			localid = vty->local;
            slot_id = vty->slotindex;
			id = (unsigned)vty->index_sub;
			type= 1;
		}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
            index = vty->index;
            localid = vty->local;
            slot_id = vty->slotindex;
            id = (unsigned)vty->index_sub;
			type= 1;
        }
        DBusConnection *dcli_dbus_connection = NULL;
        ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

		RadioList_Head = set_radio_max_rate_cmd_set_max_rate(localid,index,dcli_dbus_connection,type,id,rate,&count,&ret,&ret1,&num,&mode,list1);	
		
		if(type==0)
			{
				if(ret==0)
					{
						if(ret1==-1)
							{
								vty_out(vty,"<error> an unexpect error\n");
							}
						else
							{
								vty_out(vty,"set radio %d-%d max rate successfully\n",id/L_RADIO_NUM,id%L_RADIO_NUM);
								vty_out(vty,"radio support ratenum : %d\n",num);
								vty_out(vty,"radio support ratelist : ");
								for(i=0;i<num;i++)
									{
										vty_out(vty,"%d ",list1[i]);
									}
								vty_out(vty," \n");
							}
					}
				else if(ret == RADIO_ID_NOT_EXIST)
					vty_out(vty,"<error> radio id does not exist\n");
				else if(ret == WTP_NO_SURPORT_Rate)
					{
						if (mode == 1)
							{
								vty_out(vty,"<error> wtp mode is 11b,does not support rate\n");
								vty_out(vty,"mode 11b support rate list:10 20 55 110\n");
							}
						else if (mode == 2)
							{
								vty_out(vty,"<error> wtp mode is 11a,does not support rate\n");
								vty_out(vty,"mode 11a support rate list:60 90 120 180 240 360 480 540\n");
							}
						else if (mode == 4)
						{
							vty_out(vty,"<error> wtp mode is 11g,does not support rate\n");
							vty_out(vty,"mode 11g support rate list:60 90 120 180 240 360 480 540\n");
						}
						else if (mode == 10)
						{
							vty_out(vty,"<error> wtp mode is 11an,does not support rate\n");
							vty_out(vty,"mode 11an support rate list:60 90 120 180 240 360 480 540\n");
						}
						else if (mode == 12)
						{
							vty_out(vty,"<error> wtp mode is 11gn,does not support rate\n");
							vty_out(vty,"mode 11gn support rate list:60 90 120 180 240 360 480 540\n");
						}
						else if (mode == 26)
						{
							vty_out(vty,"<error> wtp mode is 11a/an,does not support rate\n");
							vty_out(vty,"mode 11a/an support rate list:60 90 120 180 240 360 480 540\n");
						}
						else if (mode == 44)
						{
							vty_out(vty,"<error> wtp mode is 11g/gn,does not support rate\n");
							vty_out(vty,"mode 11g/gn support rate list:60 90 120 180 240 360 480 540\n");
						}				
						else if (mode == 5)
						{
							vty_out(vty,"<error> wtp mode is 11b/g,does not support rate\n");
							vty_out(vty,"mode 11b/g support rate list:10 20 55 60 90 110 120 180 240 360 480 540\n");
						}
						else if (mode == 13)
						{
							vty_out(vty,"<error> wtp mode is 11b/g/n,does not support rate\n");
							vty_out(vty,"mode 11b/g/n support rate list:10 20 60 90 110 120 180 240 360 480 540\n");
						}
						
						else
							{
								vty_out(vty,"<error> wtp radio does not support this rate,please check first\n");
							}
					}
				else if(ret == RADIO_IS_DISABLE)
					vty_out(vty,"<error> radio is disable, please enable it first\n");
				else if(ret == RADIO_MODE_IS_11N)
					vty_out(vty,"<error> radio mode is 11n,not allow to set rate\n");
				else if(ret == RADIO_SUPPORT_RATE_EMPTY)
					vty_out(vty,"<error> radio list is empty\n");
				else if(ret == RADIO_SUPPORT_RATE_NOT_EXIST)/*use in delete support ratelist,maybe later*/
					vty_out(vty,"<error> radio support rate does not exist\n");
				else if((ret == WTP_NO_SURPORT_TYPE)||(ret == RADIO_SUPPORT_RATE_CONFLICT))
					vty_out(vty,"<error> radio type is conflict, please check it first\n");
				else
					vty_out(vty,"<error>  %d\n",ret);				
			}
		else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set radio max rate successfully\n",id);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}	
		return CMD_SUCCESS;
}

#else
/* book add 11n rate , 2010-10-21 */
DEFUN(set_radio_max_rate_cmd_func,
	  set_radio_max_rate_cmd,
	  "set max rate RATE",
	  "set radio rate\n"
	  "set radio rate\n"
	  "set radio max rate\n"
	  "Radio rate value: 11b/g: (10,20,55,60,90,110,120,180,240,360,480,540) 11n: (65,130,135,150,195,260,270,300,390,405,450,520,540,585,600,650,780,810,900,1040,1080,1170,1200,1215,1300,1350,1500,1620,1800,2160,2400,2430,2700,3000)\n"
	 )
{
	
	int mode,rate; 
	unsigned int radio_id = 0;
	int ret = 0;
	int num=0;
	int i = 0;
	int list1[RADIO_RATE_LIST_LEN];
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
    int ret1 = 0;
    struct dcli_n_rate_info nRateInfo;
    unsigned char channel = 0;
    int result = 0;
    struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
	
	/*parse the rate */
	ret = parse_int_ID((char *)argv[0],&rate);
	if (ret != WID_DBUS_SUCCESS)
	{
		vty_out(vty,"<error> input parameter should be <10-540>\n");
		vty_out(vty,"You should choose Radio rate value:\n 11b/g: (10,20,55,60,90,110,120,180,240,360,480,540)\n 11n: (65,130,135,150,195,260,270,300,390,405,450,520,540,585,600,650,780,810,900, \n\t1040,1080,1170,1200,1215,1300,1350,1500,1620,1800,2160,2400,2430,2700,3000)\n");
		return CMD_SUCCESS;
	}
	
	
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

    set_radio_max_rate_cmd_set_max_rate_v1(localid,index,dcli_dbus_connection,radio_id,rate,&ret,&ret1,&num,&mode,list1,&nRateInfo,&channel,0);	
	//printf("mcs = %d, cwmode = %d, gi = %d, mode = %d\n",nRateInfo.mcs,nRateInfo.cwmode,nRateInfo.guard_interval,mode);
		
	if(ret == 0)
	{
	    if(mode<8)
	    {
			if(ret1==-1)
			{
				vty_out(vty,"<error> an unexpect error\n");
			}
			else 
			{
				vty_out(vty,"set radio %d-%d max rate successfully\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM);
				vty_out(vty,"radio support ratenum : %d\n",num);
				vty_out(vty,"radio support ratelist : ");
				for(i=0;i<num;i++)
				{
					vty_out(vty,"%d ",list1[i]);
				}
				vty_out(vty," \n");
			}
		}
		else
		{
		    if((nRateInfo.mcs<0) || (nRateInfo.cwmode<0) || (nRateInfo.guard_interval<0))
		    {
		        result = 1;
		    }
		    if(result == 1)
		    {
			    char sel[PATH_LEN] = {0};
    			//fflush(stdin);
    			vty_out(vty,"Setting of rate need change parameters shown below:\n");
    			if(nRateInfo.mcs < 0)
    			    vty_out(vty,"mcs : %d\n", nRateInfo.mcs+1000);    
    			if(nRateInfo.cwmode < 0)
    			    vty_out(vty,"cwmode : %s\n", ((nRateInfo.cwmode+1000) == 0)?"ht20":"ht40");
    			if(nRateInfo.guard_interval < 0)
    			    vty_out(vty,"guard interval : %d\n", nRateInfo.guard_interval+1000);
    			if(channel != 0)
    			    vty_out(vty,"channel : %d\n", channel);
    			vty_out(vty,"Do you steel want to set rate? [yes/no]:\n");
    			fscanf(stdin, "%s", sel);
    			while(1){
    				if(!strncasecmp("yes", sel, strlen(sel))){
    			        result = set_11n_rate_paras(localid,index,dcli_dbus_connection,nRateInfo.mcs,nRateInfo.cwmode,nRateInfo.guard_interval,radio_id,channel);
    			        if(result == 0)
    			            vty_out(vty,"set radio rate successful!!!\n");
    			        else
    			            vty_out(vty,"failed to set radio rate\n");
    					break;
    				}
    				else if(!strncasecmp("no", sel, strlen(sel))){
    				    vty_out(vty,"user stop setting radio rate\n");
    					break;
    				}
    				else{
    					vty_out(vty,"% Please answer 'yes' or 'no'.\n");
    					vty_out(vty,"Do you steel want to set rate? [yes/no]:\n");
    					memset(sel, 0, PATH_LEN);
    					fscanf(stdin, "%s", sel);
    				}
    			}
    	    }
			
		}
	}
	else if(ret == RADIO_ID_NOT_EXIST)
		vty_out(vty,"<error> radio id does not exist\n");
	else if(ret == WTP_NO_SURPORT_Rate)
	{
		if (mode == 1)
		{
			vty_out(vty,"<error> wtp mode is 11b,does not support rate\n");
			vty_out(vty,"mode 11b support rate list:10 20 55 110\n");
		}
		else if (mode == 2)
		{
			vty_out(vty,"<error> wtp mode is 11a,does not support rate\n");
			vty_out(vty,"mode 11a support rate list:60 90 120 180 240 360 480 540\n");
		}
		else if (mode == 4)
		{
			vty_out(vty,"<error> wtp mode is 11g,does not support rate\n");
			vty_out(vty,"mode 11g support rate list:60 90 120 180 240 360 480 540\n");
		}
		else if (mode == 10)
		{
			vty_out(vty,"<error> wtp mode is 11an,does not support rate\n");
			vty_out(vty,"mode 11an support rate list:60 90 120 180 240 360 480 540\n");
		}
		else if (mode == 12)
		{
			vty_out(vty,"<error> wtp mode is 11gn,does not support rate\n");
			vty_out(vty,"mode 11gn support rate list:60 90 120 180 240 360 480 540\n");
		}
		else if (mode == 26)
		{
			vty_out(vty,"<error> wtp mode is 11a/an,does not support rate\n");
			vty_out(vty,"mode 11a/an support rate list:60 90 120 180 240 360 480 540\n");
		}
		else if (mode == 44)
		{
			vty_out(vty,"<error> wtp mode is 11g/gn,does not support rate\n");
			vty_out(vty,"mode 11g/gn support rate list:60 90 120 180 240 360 480 540\n");
		}				
		else if (mode == 5)
		{
			vty_out(vty,"<error> wtp mode is 11b/g,does not support rate\n");
			vty_out(vty,"mode 11b/g support rate list:10 20 55 60 90 110 120 180 240 360 480 540\n");
		}
		else if (mode == 13)
		{
			vty_out(vty,"<error> wtp mode is 11b/g/n,does not support rate\n");
			vty_out(vty,"mode 11b/g/n support rate list:10 20 60 90 110 120 180 240 360 480 540\n");
		}
		else
		{
			vty_out(vty,"<error> wtp radio does not support this rate,please check first\n");
		}
	}
	else if(ret == RADIO_IS_DISABLE)
		vty_out(vty,"<error> radio is disable, please enable it first\n");
	else if(ret == RADIO_MODE_IS_11N)
		vty_out(vty,"<error> radio stream number is not support this rate\n");
	else if(ret == WTP_IS_NOT_BINDING_WLAN_ID)
		vty_out(vty,"<error> wtp is not binding wlan\n");
	else if(ret == RADIO_SUPPORT_RATE_EMPTY)
		vty_out(vty,"<error> radio list is empty\n");
	else if(ret == RADIO_SUPPORT_RATE_NOT_EXIST)/*use in delete support ratelist,maybe later*/
		vty_out(vty,"<error> radio support rate does not exist\n");
	else if((ret == WTP_NO_SURPORT_TYPE)||(ret == RADIO_SUPPORT_RATE_CONFLICT))
		vty_out(vty,"<error> radio type is conflict, please check it first\n");
	else
		vty_out(vty,"<error>  %d\n",ret);			

	/*printf("ok,ret %d\n",ret);*/
	return CMD_SUCCESS;

}
#endif
#if _GROUP_POLICY
DEFUN(set_radio_mode_cmd_func,
	 set_radio_mode_cmd,
	  /*"mode (11b|11a|11g|11n|11b/g)",*/
	  "mode (11a|11b|11g|11gn|11g/gn|11b/g|11b/g/n|11a/an|11an)",   
	  SERVICE_STR
	  "Radio mode value\n"
	"you can set 11b|11a|11g|11gn|11g/gn|11b/g/n|11a/an|11an|11b/g\n" 
	 )
{

	unsigned int modetmp, mode = 0;
	int num=0;
	int i = 0;
	int list1[RADIO_RATE_LIST_LEN];
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	
	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
	//struct RadioRate_list *list1= NULL;
	struct RadioRate_list *radiorate_list = NULL;
	 	

	/*make robust code*/
	
	if(!strcmp(argv[0],"11b"))
	{
		mode |= 0x01;	
	}
	else if(!strcmp(argv[0],"11a"))
	{
		mode |= 0x02;
	}
	else if(!strcmp(argv[0],"11g"))
	{
		mode |= 0x04;
	}
	else if(!strcmp(argv[0],"11n"))
	{
		mode |= 0x08;
	}
	else if(!strcmp(argv[0],"11b/g"))
	{
		mode |= 0x01;
		mode |= 0x04;
	}
	else if(!strcmp(argv[0],"11b/g/n"))
	{
		mode |= 0x01;
		mode |= 0x04;
		mode |= 0x08;
	}
	else if(!strcmp(argv[0],"11an")) //fengwenchao change "11a/n" to "11a/an" 20120131
	{
		mode |= 0x02;
		mode |= 0x08;
	}
	/*fengwenchao add for GM,20111109*/
	else if(!strcmp(argv[0],"11gn"))
	{
		mode |= 0x04;
		mode |= 0x08;
	}
	else if(!strcmp(argv[0],"11a/an"))
	{
		mode |= 0x02;
		mode |= 0x08;
		mode |= 0x10;
	}	
	else if(!strcmp(argv[0],"11g/gn"))
	{
		mode |= 0x04;
		mode |= 0x08;
		mode |= 0x20;
	}		
	/*fengwenchao add end*/ 
	else
	{
		modetmp = 0;
		modetmp = atoi(argv[0]);
		if((modetmp == 1)||(modetmp == 2)||(modetmp == 4)||(modetmp == 5)||(modetmp == 8)||(modetmp == 10)||(modetmp == 13)
		||(modetmp == 12)||(modetmp == 26)||(modetmp == 44))  /*fengwenchao modify 20111109 for GM*/
		{
			mode |= modetmp;
		}
		else
		{
			vty_out(vty,"<error> parameter %s error\n",argv[0]);
			return CMD_SUCCESS;
		}
	}

	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		
	RadioList_Head = set_radio_mode_cmd_11a_11b_11g_11bg_11bgn_11an(localid,index,dcli_dbus_connection,type,id,mode,list1,&count,&ret);

	if(type==0)
		{
			if(ret == 0)
				{
					vty_out(vty,"set radio %d-%d mode successfully\n",id/L_RADIO_NUM,id%L_RADIO_NUM);
				}
			else if(ret == -1)
				{
					vty_out(vty,"<error> an unexpect error\n");
				}
			else if(ret == RADIO_ID_NOT_EXIST)
					vty_out(vty,"<error> radio id did not exist\n");
			else if(ret == WTP_NO_SURPORT_TYPE)
				{
					vty_out(vty,"<error> wtp radio does not support this type\n");
					vty_out(vty,"wtp surport radio type list: 11b 11g 11b/g 11a/n 11b/g/n\n");
				}
			else if(ret == RADIO_IS_DISABLE)
					vty_out(vty,"<error> radio is disable,please enable it first\n");
			else if(ret == RADIO_MODE_IS_11N)
					vty_out(vty,"<error> radio mode not allow to set with 11n\n");
			else
					vty_out(vty,"<error>  %d\n",ret);
		}	
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set radio support ratelist successfully\n",id);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}
	return CMD_SUCCESS;
}

#else
DEFUN(set_radio_mode_cmd_func,
	  set_radio_mode_cmd,
	  /*"mode (11b|11a|11g|11n|11b/g)",*/
	  "mode (11a|11b|11g|11gn|11g/gn|11b/g|11b/g/n|11a/an|11an)",   
	  SERVICE_STR
	  "Radio mode value\n"
	"you can set 11b|11a|11g|11gn|11g/gn|11b/g/n|11a/an|11an|11b/g\n" 
	 )
{
	unsigned int radio_id; 
	unsigned int	modetmp, mode = 0;
	int ret;
	int num=0;
	int i = 0;
	int list1[RADIO_RATE_LIST_LEN];
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;	
	/*make robust code*/
	
	if(!strcmp(argv[0],"11b"))
	{
		mode |= 0x01;	
	}
	else if(!strcmp(argv[0],"11a"))
	{
		mode |= 0x02;
	}
	else if(!strcmp(argv[0],"11g"))
	{
		mode |= 0x04;
	}
	else if(!strcmp(argv[0],"11n"))
	{
		mode |= 0x08;
	}
	else if(!strcmp(argv[0],"11b/g"))
	{
		mode |= 0x01;
		mode |= 0x04;
	}
	else if(!strcmp(argv[0],"11b/g/n"))
	{
		mode |= 0x01;
		mode |= 0x04;
		mode |= 0x08;
	}
	else if(!strcmp(argv[0],"11an")) //fengwenchao change "11a/n" to "11a/an" 20120131
	{
		mode |= 0x02;
		mode |= 0x08;
	}
	/*fengwenchao add for GM,20111109*/
	else if(!strcmp(argv[0],"11gn"))
	{
		mode |= 0x04;
		mode |= 0x08;
	}
	else if(!strcmp(argv[0],"11a/an"))
	{
		mode |= 0x02;
		mode |= 0x08;
		mode |= 0x10;
	}	
	else if(!strcmp(argv[0],"11g/gn"))
	{
		mode |= 0x04;
		mode |= 0x08;
		mode |= 0x20;
	}		
	/*fengwenchao add end*/ 
	else
	{
		modetmp = 0;
		modetmp = atoi(argv[0]);
		if((modetmp == 1)||(modetmp == 2)||(modetmp == 4)||(modetmp == 5)||(modetmp == 8)||(modetmp == 10)||(modetmp == 13)
		||(modetmp == 12)||(modetmp == 26)||(modetmp == 44))  /*fengwenchao modify 20111109 for GM*/
		{
			mode |= modetmp;
		}
		else
		{
			vty_out(vty,"<error> parameter %s error\n",argv[0]);
			return CMD_SUCCESS;
		}
	}

	//radio_id = (int)vty->index;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_MODE);
		

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_SET_MODE);*/
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_UINT32,&mode,
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
		{
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&num);
			if(num > 20)
			{
				vty_out(vty,"<error> an unexpect error\n");
				return CMD_SUCCESS;
			}
			for(i=0;i<num;i++)
			{
				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&list1[i]);
			}
			vty_out(vty,"set radio %d-%d mode successfully\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM);
			/*vty_out(vty,"radio support ratenum : %d\n",num);
			//vty_out(vty,"radio support ratelist : ");
			//for(i=0;i<num;i++)
			//{
			//	vty_out(vty,"%d ",list1[i]);
			//}
			//vty_out(vty," \n");*/

		}
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio id did not exist\n");
		else if(ret == WTP_NO_SURPORT_TYPE)
		{
			vty_out(vty,"<error> wtp radio does not support this type\n");
			vty_out(vty,"wtp surport radio type list: 11b 11g 11b/g 11a/an 11an 11b/g/n 11gn 11g/gn\n");  /*fengwenchao modify 20111109 for GM*/
		}
		else if(ret == RADIO_IS_DISABLE)
			vty_out(vty,"<error> radio is disable,please enable it first\n");
		else if(ret == RADIO_MODE_IS_11N)
			vty_out(vty,"<error> radio mode not allow to set with 11n\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;

}
#endif

DEFUN(set_radio_management_frame_rate_cmd_func,
	  set_radio_management_frame_rate_cmd,
	  "set wlan WLANID  rate (54|48|36|24|18|12|9|6|11|5.5|2|1) for type .TYPES",
	    SERVICE_STR
	   "Set radio management frame rate\n"
	   "base wlan\n"
	   "applyed wlan ID\n"
	   "you can set 54|48|36|24|18|12|9|6|11|5.5|2|1 M/s\n"
	   "rate\n"   "rate\n"   "rate\n"   "rate\n"   "rate\n"   "rate\n"   "rate\n"   "rate\n"   "rate\n"   "rate\n"   "rate\n"   "rate\n"  
	   "Set radio management frame rate\n"
	   "Set radio management frame rate\n"
	   "massage type"
	   "you can set (all beacon probe_request probe_response auth assoc_request assoc_response reassoc_request reassoc_response deauth disassoc)\n"
	 )
{
	unsigned int radio_id,wlanid,type = 0,rate = 0; 
	int ret,num = 0;
	unsigned int i;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;	
	
	num = argc -2;
	if(num <=  0){
		vty_out(vty,"<error> type  you want (like  all  beacon  probe_request  probe_response  auth  assoc_request  assoc_response   reassoc_request  reassoc_response  deauth  disassoc)\n");
		return CMD_SUCCESS;		
	}
	for (i = 2; i < argc; i++)
	{
		if(!strcmp(argv[i],"all"))
		{
			type  = 0x100;
			break;
		}
		else if(!strcmp(argv[i],"beacon"))
		{
			type |=  0x80;
			
		}
		else if(!strcmp(argv[i],"probe_request"))
		{
			type |= 0x40;
			
		}
		else if(!strcmp(argv[i],"probe_response"))
		{
			type |=0x20;
			
		}
		else if(!strcmp(argv[i],"auth"))
		{
			type |= 0x10;
		}
		else if((!strcmp(argv[i],"assoc_request")) || (!strcmp(argv[i],"reassoc_request")))
		{
			type |= 0x08;
			
		}
		else if((!strcmp(argv[i],"assoc_response")) || (!strcmp(argv[i],"reassoc_response")))
		{
			type |= 0x04;
			
		}	
		else if(!strcmp(argv[i],"deauth"))
		{
			type |= 0x02;
			
		}	
		else if(!strcmp(argv[i],"disassoc"))
		{
			type |= 0x01;
			
		}
		else
		{
		
			vty_out(vty,"<error> parameter %s error\n",argv[i]);
			return CMD_SUCCESS;
		}

	}
	if(!strcmp(argv[1],"54"))
	{
		rate = MGMT_RATE_54M;	
	}
	else if(!strcmp(argv[1],"48"))
	{
		rate = MGMT_RATE_48M;
	}
	else if(!strcmp(argv[1],"36"))
	{
		rate = MGMT_RATE_36M;
	}
	else if(!strcmp(argv[1],"24"))
	{
		rate = MGMT_RATE_24M;
	}
	else if(!strcmp(argv[1],"18"))
	{
		rate = MGMT_RATE_18M;
	}
	else if(!strcmp(argv[1],"12"))
	{
		rate = MGMT_RATE_12M;
	}
	else if(!strcmp(argv[1],"9")) 
	{
		rate = MGMT_RATE_9M;
	}
	else if(!strcmp(argv[1],"6"))
	{
		rate = MGMT_RATE_6M;
	}
	else if(!strcmp(argv[1],"11"))
	{
		rate = MGMT_RATE_11M;
	}	
	else if(!strcmp(argv[1],"5.5"))
	{
		rate = MGMT_RATE_5_5M;
	}	
	else if(!strcmp(argv[1],"2"))
	{
		rate = MGMT_RATE_2M;
	}
	else if(!strcmp(argv[1],"1"))
	{
		rate = MGMT_RATE_1M;
	}
	else
	{
		
		vty_out(vty,"<error> parameter %s error\n",argv[1]);
		return CMD_SUCCESS;
	}
	
	ret = parse_int_ID((char *)argv[0],&wlanid);    
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}
	
	int index = 0;
	int localid = 1;
          int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE)
	{
		index = 0;			
		radio_id = (unsigned int)vty->index;
	}
	else if(vty->node == HANSI_RADIO_NODE)
	{
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (unsigned int)vty->index_sub;
	}
	else if (vty->node == LOCAL_HANSI_RADIO_NODE)
	{
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (unsigned int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_MGMT_RATE_BASE_WLAN);
		

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_SET_MODE);*/
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_UINT32,&rate,
							 DBUS_TYPE_UINT32,&wlanid,
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
		vty_out(vty,"set radio %d-%d mode successfully\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM);

	}
	else if(ret == WTP_NO_SURPORT_Rate)
	{
		vty_out(vty,"<error> not support this rate!\n");
	}
	else if(ret == WTP_IF_NOT_BE_BINDED)
	{
		vty_out(vty,"<failed> radio should bind this wlan first !\n");
	}
	else if(ret == WTP_NO_SURPORT_Mode)
	{
		vty_out(vty,"wtp no  support  the mode\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;

}

DEFUN( radio_clear_rate_for_wlan__cmd_func,
	  radio_clear_rate_for_wlan_cmd,
	  "clear rate for wlan WLANID",
	    SERVICE_STR
	   "clear all rate"
	   "clear all rate"
	   "apply wlan"
	   " apply wlan ID"
	   )
{
	unsigned int radio_id,wlanid; 
	int ret;
	unsigned int i;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;	
		
	ret = parse_int_ID((char *)argv[0],&wlanid);    
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}
	
	int index = 0;
	int localid = 1;
          int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE)
	{
		index = 0;			
		radio_id = (int)vty->index;
	}
	else if(vty->node == HANSI_RADIO_NODE)
	{
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}
	else if (vty->node == LOCAL_HANSI_RADIO_NODE)
	{
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
      radio_id = (int)vty->index_sub;
    }
	
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
    ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
   ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
   ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_CLAER_RATE_FOR_WLAN);
		

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_SET_MODE);*/
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_UINT32,&wlanid,
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
		vty_out(vty,"clear rate successfully\n");

	}

	else if(ret == WTP_IF_NOT_BE_BINDED)
	{
		vty_out(vty,"radio show bind this wlan first !\n");
	}
	
	else
		vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;

}


#if _GROUP_POLICY

/*added end*/
DEFUN(set_radio_beaconinterval_cmd_func,
	  set_radio_beaconinterval_cmd,
	  "beaconinterval VALUE",             //fengwenchao change <25-1000> to VALUE  ,20110504
	  "set radio beaconinterval\n"
	  "Radio beaconinterval value <25-1000>\n"      //fengwenchao modify 20110504
	 )
{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned short beaconinterval= 0;
	
	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 	
	
	ret = parse_short_ID((char *)argv[0],&beaconinterval);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		   if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
		   }
		   else{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		   }
		return CMD_SUCCESS;
	}
	if ((beaconinterval < 25)||(beaconinterval > 1000)) 
	{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}
	/*make robust code*/

	if((beaconinterval< 25)||(beaconinterval> 1000))
	{
		vty_out(vty,"<error> input parameter should be 25 to 1000\n");
		return CMD_SUCCESS;
	}
	//radio_id = (int)vty->index;
	

	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = set_radio_beaconinterval_cmd_beaconinterval(localid,index,dcli_dbus_connection,type,id,beaconinterval,
	&count,&ret);	

	if(type==0)
		{
			if(ret == 0)
				vty_out(vty,"Radio %d-%d set beacon interval %s successfully .\n",id/L_RADIO_NUM,id%L_RADIO_NUM,argv[0]);
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio id does not exist\n");
			else if(ret == RADIO_IS_DISABLE)
				vty_out(vty,"<error> radio is disable, please enable it first\n");
			else
				vty_out(vty,"<error>  %d\n",ret);
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d Radio set beacon interval %s successfully .\n",id,argv[0]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}

	return CMD_SUCCESS;
}

#else
/*added end*/
DEFUN(set_radio_beaconinterval_cmd_func,
	  set_radio_beaconinterval_cmd,
	  "beaconinterval VALUE",                  //fengwenchao change <25-1000> to VALUE  ,20110504
	  "set radio beaconinterval\n"
	  "Radio beaconinterval value  <25-1000>\n"      //fengwenchao modify 20110504
	 )
{
	unsigned int radio_id; 
	unsigned short beaconinterval;
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;	
    /*beaconinterval= atoi(argv[0]);*/
	
	ret = parse_short_ID((char *)argv[0],&beaconinterval);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		   if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
		   }
		   else{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		   }
		return CMD_SUCCESS;
	}
	if ((beaconinterval < 25)||(beaconinterval > 1000)) 
	{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}
	/*make robust code*/

	if((beaconinterval< 25)||(beaconinterval> 1000))
	{
		vty_out(vty,"<error> input parameter should be 25 to 1000\n");
		return CMD_SUCCESS;
	}
	//radio_id = (int)vty->index;
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_BEACON);
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_SET_BEACON);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_UINT16,&beaconinterval,
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
			vty_out(vty,"Radio %d-%d set beacon interval %s successfully .\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0]);
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio id does not exist\n");
		else if(ret == RADIO_IS_DISABLE)
			vty_out(vty,"<error> radio is disable, please enable it first\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;

}
#endif
#if _GROUP_POLICY
DEFUN(set_radio_fragmentation_cmd_func,
	  set_radio_fragmentation_cmd,
	  "fragmentation VALUE",                   //fengwenchao change <256-2346> to VALUE  20110504
	  "set radio fragmentation\n"
	  "Radio max fragmentation value <256-2346>\n"      //fengwenchao modify 20110504
	 )
{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned short fragmentation= 0;

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 	

	ret = parse_short_ID((char *)argv[0],&fragmentation);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
       if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
       }
	   else{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
	   }
		return CMD_SUCCESS;
	}
	if ((fragmentation < 256)||(fragmentation > 2346)) 
	{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}
	/*make robust code*/

	if((fragmentation< 256)||(fragmentation> 2346))
	{
		vty_out(vty,"<error> input parameter should be 256-2346\n");
		return CMD_SUCCESS;
	}
	//radio_id = (int)vty->index;
	
	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index;
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = set_radio_fragmentation_cmd_fragmentation(localid,index,dcli_dbus_connection,type,id,fragmentation,
	&count,&ret);	

	if(type==0)
		{
			if(ret == 0)
				vty_out(vty,"Radio %d-%d set fragmentation size %s successfully .\n",id/L_RADIO_NUM,id%L_RADIO_NUM,argv[0]);
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio id does not exist\n");
			else if(ret == RADIO_IS_DISABLE)
				vty_out(vty,"<error> radio is disable, please enable it first\n");
			else
				vty_out(vty,"<error>  %d\n",ret);
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d Radio set fragmentation size %s successfully .\n",id,argv[0]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}	
	return CMD_SUCCESS;

}

#else
DEFUN(set_radio_fragmentation_cmd_func,
	  set_radio_fragmentation_cmd,
	  "fragmentation VALUE",                       //fengwenchao change <256-2346> to VALUE  20110504
	  "set radio fragmentation\n" 
	  "Radio max fragmentation value <256-2346>\n"            //fengwenchao modify 20110504
	 )
{
	unsigned int radio_id; 
	unsigned short fragmentation;
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;	
    /*fragmentation= atoi(argv[0]);*/
	ret = parse_short_ID((char *)argv[0],&fragmentation);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
       if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
       }
	   else{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
	   }
		return CMD_SUCCESS;
	}
	if ((fragmentation < 256)||(fragmentation > 2346)) 
	{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}
	/*make robust code*/

	if((fragmentation< 256)||(fragmentation> 2346))
	{
		vty_out(vty,"<error> input parameter should be 256-2346\n");
		return CMD_SUCCESS;
	}
	//radio_id = (int)vty->index;
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_FRAGMENTATION);
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_SET_FRAGMENTATION);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_UINT16,&fragmentation,
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
			vty_out(vty,"Radio %d-%d set fragmentation size %s successfully .\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0]);
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio id does not exist\n");
		else if(ret == RADIO_IS_DISABLE)
			vty_out(vty,"<error> radio is disable, please enable it first\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;

}
#endif
#if _GROUP_POLICY

DEFUN(set_radio_dtim_cmd_func,
	  set_radio_dtim_cmd,
	  "dtim VALUE",              //fengwenchao change <1-15> to VALUE
	  "set radio dtim\n"
	  "Radio dtim value <1-15>\n"       //fengwenchao modify 20110504
	 )
{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned char dtim = 0;

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 

	ret = parse_char_ID((char *)argv[0],&dtim);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
       if(ret == WID_ILLEGAL_INPUT){
			vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
       }
	   else{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
	   }
		return CMD_SUCCESS;
	}

	if((dtim< 1)||(dtim> 15))
	{
		vty_out(vty,"<error> input parameter should be 1 to 15\n");/*sz amend*/
		return CMD_SUCCESS;
	}

	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = set_radio_dtim_cmd_dtim(localid,index,dcli_dbus_connection,type,id,dtim,
	&count,&ret);	

	if(type==0)
		{
			if(ret == 0)
				vty_out(vty,"radio %d-%d set dtim %s successfully .\n",id/L_RADIO_NUM,id%L_RADIO_NUM,argv[0]);
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio id does not exist\n");
			else if(ret == RADIO_IS_DISABLE)
				vty_out(vty,"<error> radio is disable, please enable it first\n");
			else
				vty_out(vty,"<error>  %d\n",ret);
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d radio set dtim %s successfully .\n",id,argv[0]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}	
	return CMD_SUCCESS;
}

#else
DEFUN(set_radio_dtim_cmd_func,
	  set_radio_dtim_cmd,
	  "dtim VALUE",           //fengwenchao change <1-15> to VALUE
	  "set radio dtim\n"
	  "Radio dtim value <1-15>\n"       //fengwenchao modify 20110504
	 )
{
	unsigned int radio_id; 
	unsigned char dtim;
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;	
    /*dtim= atoi(argv[0]);*/
	ret = parse_char_ID((char *)argv[0],&dtim);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
       if(ret == WID_ILLEGAL_INPUT){
			vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
       }
	   else{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
	   }
		return CMD_SUCCESS;
	}
	
	/*make robust code*/

	if((dtim< 1)||(dtim> 15))
	{
		vty_out(vty,"<error> input parameter should be 1 to 15\n");/*sz amend*/
		return CMD_SUCCESS;
	}
	//radio_id = (int)vty->index;
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_DTIM);
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_SET_DTIM);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_BYTE,&dtim,
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
			vty_out(vty,"radio %d-%d set dtim %s successfully .\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0]);
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio id does not exist\n");
		else if(ret == RADIO_IS_DISABLE)
			vty_out(vty,"<error> radio is disable, please enable it first\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;

}
#endif
#if _GROUP_POLICY
DEFUN(set_radio_rtsthreshold_cmd_func,
	  set_radio_rtsthreshold_cmd,
	  "rtsthreshold VALUE",        //fengwenchao change <256-2346> to VALUE  20110504
	  "set rtsthreshold \n"
	  "Radio rtsthreshold value <256-2346>\n"       //fengwenchao modify 20110504
	 )
{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned short rtsthre = 0;

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 

	ret = parse_short_ID((char *)argv[0],&rtsthre);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
       if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
       }
	   else{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
	   }
		return CMD_SUCCESS;
	}

	/*make robust code*/

	if((rtsthre < 256)||(rtsthre > 2346))
	{
		vty_out(vty,"<error> input parameter should be 256 to 2346\n");
		return CMD_SUCCESS;
	}

	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = set_radio_rtsthreshold_cmd_rtsthreshold(localid,index,dcli_dbus_connection,type,id,rtsthre,
	&count,&ret);	

	if(type==0)
		{
			if(ret == 0)
				vty_out(vty,"Radio %d-%d set rtsthreshold %s successfully .\n",id/L_RADIO_NUM,id%L_RADIO_NUM,argv[0]);
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio id does not exist\n");
			else if(ret == RADIO_IS_DISABLE)
				vty_out(vty,"<error> radio is disable, please enable it first\n");
			else
				vty_out(vty,"<error>  %d\n",ret);
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d Radio set rtsthreshold %s successfully .\n",id,argv[0]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}	
	return CMD_SUCCESS;

}

#else
DEFUN(set_radio_rtsthreshold_cmd_func,
	  set_radio_rtsthreshold_cmd,
	  "rtsthreshold VALUE",                   //fengwenchao change <256-2346> to VALUE  20110504
	  "set rtsthreshold \n"
	  "Radio rtsthreshold value <256-2346>\n"         //fengwenchao modify 20110504
	 )


{
	unsigned int radio_id; 
	unsigned short rtsthre;
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;	
	
    /*rtsthre= atoi(argv[0]);*/
	ret = parse_short_ID((char *)argv[0],&rtsthre);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
       if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
       }
	   else{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
	   }
		return CMD_SUCCESS;
	}

	/*make robust code*/

	if((rtsthre < 256)||(rtsthre > 2346))
	{
		vty_out(vty,"<error> input parameter should be 256 to 2346\n");
		return CMD_SUCCESS;
	}


	//radio_id = (int)vty->index;
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_RTSTHROLD);
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_SET_RTSTHROLD);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_UINT16,&rtsthre,
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
			vty_out(vty,"Radio %d-%d set rtsthreshold %s successfully .\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0]);
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio id does not exist\n");
		else if(ret == RADIO_IS_DISABLE)
			vty_out(vty,"<error> radio is disable, please enable it first\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;

}
#endif
#if _GROUP_POLICY
DEFUN(set_radio_service_cmd_func,
	  set_radio_service_cmd,
	  "radio (enable|disable)",
	  SERVICE_STR
	  "radio service value enable/disable\n"
	 )

{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned char status = 0;

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 

	if (!strcmp(argv[0],"enable"))
	{
		status = 1;	
	}		
	else if (!strcmp(argv[0],"disable"))
	{
		status = 2;
	}
	else
	{
		vty_out(vty,"<error>  input patameter should only be 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}

	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = set_radio_service_cmd_radio_enable_disable(localid,index,dcli_dbus_connection,type,id,status,
	&count,&ret);	

	if(type==0)
		{
			if(ret == 0)
				vty_out(vty,"Radio %d-%d set radio status %s successfully .\n",id/L_RADIO_NUM,id%L_RADIO_NUM,argv[0]);
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio id does not exist\n");
			else if(ret ==	WTP_NOT_IN_RUN_STATE)
				vty_out(vty,"<error> wtp is not in run state\n");	
			else
				vty_out(vty,"<error>  %d\n",ret);
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d Radio set radio status %s successfully .\n",id,argv[0]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}	
	return CMD_SUCCESS;

}

#else
DEFUN(set_radio_service_cmd_func,
	  set_radio_service_cmd,
	  "radio (enable|disable)",
	  SERVICE_STR
	  "radio service value enable/disable\n"
	 )

{
	unsigned int radio_id; 
	unsigned char status;
	int ret;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;	
	
	if (!strcmp(argv[0],"enable"))
	{
		status = 1;	
	}		
	else if (!strcmp(argv[0],"disable"))
	{
		status = 2;
	}
	else
	{
		vty_out(vty,"<error>  input patameter should only be 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}

	//radio_id = (int)vty->index;
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index;
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_STATUS);
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_SET_STATUS);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_BYTE,&status,
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
			vty_out(vty,"Radio %d-%d set radio status %s successfully .\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0]);
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio id does not exist\n");
		else if(ret ==	WTP_NOT_IN_RUN_STATE)
			vty_out(vty,"<error> wtp is not in run state\n");	
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;

}
#endif
#if _GROUP_POLICY
DEFUN(set_wds_service_cmd_func,
	  set_wds_service_cmd,
	  "wlan ID wds (enable|disable)",
	  SERVICE_STR
	  "radio wds service value enable/disable\n"
	 )

{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned char status = 0;
	unsigned char wlanid = 0;

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 
	
	ret = parse_char_ID((char *)argv[0],&wlanid);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
			    vty_out(vty,"<error> unknown id format\n");
			}
			return CMD_SUCCESS;
	}	
	if (!strcmp(argv[1],"enable"))
	{
		status = 1;	
	}		
	else if (!strcmp(argv[1],"disable"))
	{
		status = 0;
	}
	else
	{
		vty_out(vty,"<error>  input patameter should only be 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}

	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = set_wds_service_cmd_wlan_wds(localid,index,dcli_dbus_connection,type,id,wlanid,status,
	&count,&ret);	

	if(type==0)
		{
			if(ret==WLAN_ID_NOT_EXIST)
				vty_out(vty,"<error> wlan isn't existed\n");
			else if(ret==RADIO_NO_BINDING_WLAN)
				vty_out(vty,"<error> radio doesn't bind wlan %s\n",argv[0]);
			else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
				vty_out(vty,"<error> radio doesn't bind wlan %s\n",argv[0]);
			else if(ret == 0)
				vty_out(vty,"Radio %d-%d set wds status %s successfully .\n",id/L_RADIO_NUM,id%L_RADIO_NUM,argv[0]);
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio id does not exist\n");
			else if(ret ==	WTP_NOT_IN_RUN_STATE)
				vty_out(vty,"<error> wtp is not in run state\n");	
			else if(ret == WDS_MODE_BE_USED)
				{
					vty_out(vty,"<error> another wds mode be used\n");	
				}
			else
				vty_out(vty,"<error>  %d\n",ret);
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d Radio set wds status %s successfully .\n",id,argv[0]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}	

	return CMD_SUCCESS;

}

#else
DEFUN(set_wds_service_cmd_func,
	  set_wds_service_cmd,
	  "wlan ID (wds|mesh)  (enable|disable)",
	  SERVICE_STR
	  "radio wds service value enable/disable\n"
	 )

{
	unsigned int radio_id; 
	unsigned char status;
	int ret;
	unsigned char wlanid;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;	
	
	ret = parse_char_ID((char *)argv[0],&wlanid);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
			    vty_out(vty,"<error> unknown id format\n");
			}
			return CMD_SUCCESS;
	}
	if(!strcmp(argv[1],"wds")){
		
		if (!strcmp(argv[2],"enable"))
		{
			status = 1;	
		}		
		else if (!strcmp(argv[2],"disable"))
		{
			status = 0;
		}
		else
		{
			vty_out(vty,"<error>  input patameter should only be 'enable' or 'disable'\n");
			return CMD_SUCCESS;
		}
	}
	else	if(!strcmp(argv[1],"mesh")){
		
		if (!strcmp(argv[2],"enable"))
		{
			status = 3;	
		}		
		else if (!strcmp(argv[2],"disable"))
		{
			status = 2;
		}
		else
		{
			vty_out(vty,"<error>  input patameter should only be 'enable' or 'disable'\n");
			return CMD_SUCCESS;
		}
	}
	else{
			vty_out(vty,"<error>  input patameter should only be 'wds' or 'mesh'\n");
			return CMD_SUCCESS;
	}
	//radio_id = (int)vty->index;
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_WDS_STATUS);
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_SET_WDS_STATUS);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&wlanid,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_BYTE,&status,
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

		if(ret==WLAN_ID_NOT_EXIST)
			vty_out(vty,"<error> wlan isn't existed\n");
		else if(ret==RADIO_NO_BINDING_WLAN)
			vty_out(vty,"<error> radio doesn't bind wlan %s\n",argv[0]);
		else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
			vty_out(vty,"<error> radio doesn't bind wlan\n");
		else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
		{
			vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
		}
		else if(ret == 0)
			if((status&0x02) == 0){
				vty_out(vty,"Radio %d-%d set wds status %s successfully .\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0]);
			}
			else
			{
				vty_out(vty,"Radio %d-%d set mesh status %s successfully .\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0]);
			}
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio id does not exist\n");
		else if(ret ==	WTP_NOT_IN_RUN_STATE)
			vty_out(vty,"<error> wtp is not in run state\n");	
		else if(ret == WDS_MODE_BE_USED){
			if((status&0x02) == 0){
				vty_out(vty,"<error> another wds mode be used\n");
			}
			else{
				vty_out(vty,"<error> another mesh mode be used\n");
			}
		}else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;

}
#endif
#if _GROUP_POLICY

DEFUN(set_radio_preamble_cmd_func,
	  set_radio_preamble_cmd,
	  "preamble (short|long)",
	  SERVICE_STR
	  "radio preamble value short/long\n"
	  /*"you can set short/long\n"*/
	 )

{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned char preamble = 0;

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 
	
	if ((!strcmp(argv[0],"short"))||(!strcmp(argv[0],"1")))
	{
		preamble = 1;	
	}		
	else if((!strcmp(argv[0],"long"))||(!strcmp(argv[0],"0")))
	{
		preamble = 0;
	}
	else
	{
		vty_out(vty,"<error> input parameter should only be 'long' or 'short'\n");
		return CMD_SUCCESS;
	}


	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    } 
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = set_radio_preamble_cmd_preamble(localid,index,dcli_dbus_connection,type,id,preamble,
	&count,&ret);

	if(type==0)
		{
			if(ret == 0)
				vty_out(vty,"Radio %d-%d set preamble %s successfully .\n",id/L_RADIO_NUM,id%L_RADIO_NUM,argv[0]);
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio id does not exist\n");
			else if(ret == RADIO_IS_DISABLE)
				vty_out(vty,"<error> radio is disable, please enable it first\n");
			else
				vty_out(vty,"<error>  %d\n",ret);
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d Radio set preamble %s successfully .\n",id,argv[0]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}	
	return CMD_SUCCESS;

}

#else
DEFUN(set_radio_preamble_cmd_func,
	  set_radio_preamble_cmd,
	  "preamble (short|long)",
	  SERVICE_STR
	  "radio preamble value short/long\n"
	  /*"you can set short/long\n"*/
	 )

{
	unsigned int radio_id; 
	unsigned char preamble;
	int ret;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;	
	
	if ((!strcmp(argv[0],"short"))||(!strcmp(argv[0],"1")))
	{
		preamble = 1;	
	}		
	else if((!strcmp(argv[0],"long"))||(!strcmp(argv[0],"0")))
	{
		preamble = 0;
	}
	else
	{
		vty_out(vty,"<error> input parameter should only be 'long' or 'short'\n");
		return CMD_SUCCESS;
	}

	//radio_id = (int)vty->index;
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_PREAMBLE);
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_SET_PREAMBLE);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_BYTE,&preamble,
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
			vty_out(vty,"Radio %d-%d set preamble %s successfully .\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0]);
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio id does not exist\n");
		else if(ret == RADIO_IS_DISABLE)
			vty_out(vty,"<error> radio is disable, please enable it first\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;

}
#endif
#if _GROUP_POLICY
DEFUN(set_radio_longretry_cmd_func,
	  set_radio_longretry_cmd,
	  "longretry VALUE",                      //fengwenchao change <1-15> to VALUE
	  "set long retry count \n" 
	  "Radio long retry value <1-15>\n"         //fengwenchao modify 20110504
	 )


{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned char longretry = 0;

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 

	ret = parse_char_ID((char *)argv[0],&longretry);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
       if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
       }
	   else{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
	   }
		return CMD_SUCCESS;
	}
	/*make robust code*/

	if((longretry < 1)||(longretry > 15))
	{
		vty_out(vty,"<error> input parameter should be 1 to 15\n");/*sz amend*/
		return CMD_SUCCESS;
	}


	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    } 
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = set_radio_longretry_cmd_longretry(localid,index,dcli_dbus_connection,type,id,longretry,
	&count,&ret);	

	if(type==0)
		{
			if(ret == 0)
				vty_out(vty,"Radio %d-%d set long retry %s successfully .\n",id/L_RADIO_NUM,id%L_RADIO_NUM,argv[0]);
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio id does not exist\n");
			else if(ret == RADIO_IS_DISABLE)
				vty_out(vty,"<error> radio is disable, please enable it first\n");
			else
				vty_out(vty,"<error>  %d\n",ret);
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d Radio set long retry %s successfully .\n",id,argv[0]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}	
	return CMD_SUCCESS;

}

#else
DEFUN(set_radio_longretry_cmd_func,
	  set_radio_longretry_cmd,
	  "longretry VALUE",                 //fengwenchao change <1-15> to VALUE
	  "set long retry count \n"
	  "Radio long retry value <1-15>\n"           //fengwenchao modify 20110504
	 )


{
	unsigned int radio_id; 
	unsigned char longretry;
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;	
	
    /*longretry= atoi(argv[0]);*/
	ret = parse_char_ID((char *)argv[0],&longretry);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
       if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
       }
	   else{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
	   }
		return CMD_SUCCESS;
	}
	/*make robust code*/

	if((longretry < 1)||(longretry > 15))
	{
		vty_out(vty,"<error> input parameter should be 1 to 15\n");/*sz amend*/
		return CMD_SUCCESS;
	}


	//radio_id = (int)vty->index;
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_LONGRETRY);
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_SET_LONGRETRY);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_BYTE,&longretry,
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
			vty_out(vty,"Radio %d-%d set long retry %s successfully .\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0]);
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio id does not exist\n");
		else if(ret == RADIO_IS_DISABLE)
			vty_out(vty,"<error> radio is disable, please enable it first\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;

}
#endif
#if _GROUP_POLICY
DEFUN(set_radio_shortretry_cmd_func,
	  set_radio_shortretry_cmd,
	  "shortretry VALUE",                    //fengwenchao change <1-15> to VALUE
	  "set short retry count \n"
	  "Radio short retry value <1-15>\n"        //fengwenchao modify 20110504
	 )
{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned char shortretry = 0;

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 

    /*shortretry= atoi(argv[0]);*/
	ret = parse_char_ID((char *)argv[0],&shortretry);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
       if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
       }
	   else{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
	   }
		return CMD_SUCCESS;
	}
	/*make robust code*/

	if((shortretry < 1)||(shortretry > 15))
	{
		vty_out(vty,"<error> input parameter should be 1 to 15\n");/*sz amend*/
		return CMD_SUCCESS;
	}
	
	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    } 
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = set_radio_shortretry_cmd_shortretry(localid,index,dcli_dbus_connection,type,id,shortretry,
	&count,&ret);	
	if(type==0)
		{
			if(ret == 0)
				vty_out(vty,"Radio %d-%d set short retry %s successfully .\n",id/L_RADIO_NUM,id%L_RADIO_NUM,argv[0]);
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio id does not exist\n");
			else if(ret == RADIO_IS_DISABLE)
				vty_out(vty,"<error> radio is disable, please enable it first\n");
			else
				vty_out(vty,"<error>  %d\n",ret);
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d Radio set short retry %s successfully .\n",id,argv[0]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}	
	return CMD_SUCCESS;

}

#else
DEFUN(set_radio_shortretry_cmd_func,
	  set_radio_shortretry_cmd,
	  "shortretry VALUE",                      //fengwenchao change <1-15> to VALUE
	  "set short retry count \n"
	  "Radio short retry value <1-15>\n"   //fengwenchao modify 20110504
	 )


{
	unsigned int radio_id; 
	unsigned char shortretry;
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;	
	
    /*shortretry= atoi(argv[0]);*/
	ret = parse_char_ID((char *)argv[0],&shortretry);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
       if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
       }
	   else{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
	   }
		return CMD_SUCCESS;
	}
	/*make robust code*/

	if((shortretry < 1)||(shortretry > 15))
	{
		vty_out(vty,"<error> input parameter should be 1 to 15\n");/*sz amend*/
		return CMD_SUCCESS;
	}


	//radio_id = (int)vty->index;
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_SHORTRETRY);
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_SET_SHORTRETRY);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_BYTE,&shortretry,
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
			vty_out(vty,"Radio %d-%d set short retry %s successfully .\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0]);
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio id does not exist\n");
		else if(ret == RADIO_IS_DISABLE)
			vty_out(vty,"<error> radio is disable, please enable it first\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;

}
#endif

#if _GROUP_POLICY
DEFUN(set_radio_bss_max_num_cmd_func,
	  set_radio_bss_max_num_cmd,
	  "set bss wlan WLANID max_sta_num NUM",      //fengwenchao change BSSINDEX to wlan WLANID  20110512
	  "set \n"
	  "based bss\n"
	  "local bss index\n"
	  "max station number\n"
	  "<0~32767>\n"                       //fengwenchao modify 20110415
	  "max station number\n"
	  "the number value\n"
	 )
{
	int i = 0;
	int ret = 0;
	int ret2 = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	int failnum = 0;
	//unsigned int bss_index=0,max_sta_num=0;   //fengwenchao coment 20110512
	int max_sta_num = 0;   //fengwenchao add 20110512
	unsigned int wlanid = 0;     //fengwenchao add 20110512
	unsigned char wlan_wid = 0; //fengwenchao add 20110512
	unsigned int type=1;
	unsigned int bssid = 0;
	int stanum=0;  //fengwenchao change unsigned int to int 20110512
	unsigned int id = 0;
	unsigned int TYPE = 0;

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 

	ret = parse_int_ID((char *)argv[0],&wlanid);    //fengwenchao modify 20110512
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}


	ret = parse_int_ID((char *)argv[1],&max_sta_num);

	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[1]);
		return CMD_SUCCESS;
	}

	if((wlanid < 1)||(wlanid > WLAN_NUM))                            //fengwenchao modify 20110512
	{
		vty_out(vty,"<error> wlanid should be 1 to %d\n",WLAN_NUM);
		return CMD_SUCCESS;
	}


	if((max_sta_num< 0)||(max_sta_num > 32767))    //fengwenchao modify 20110415
	{
		vty_out(vty,"<error> max station number should be greater than 0,and not cross 32767\n");
		return CMD_SUCCESS;
	}
	


	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		TYPE = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		TYPE= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		TYPE= 1;
    } 
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
    
	RadioList_Head = set_radio_bss_max_num_cmd_set_bss_max_sta_num(localid,index,dcli_dbus_connection,TYPE,id,type,max_sta_num,wlanid,
		&count,&ret,&ret2,&failnum);    //fengwenchao modify 20110512

	if(TYPE==0)
		{
			if(ret == 0)
				vty_out(vty,"set bss %d max sta num %d successfully .\n",wlanid,max_sta_num);
			else if(ret==WLAN_ID_NOT_EXIST)      //fengwenchao modify 20110512
				vty_out(vty,"bss %d not exist .\n",wlanid);
			else if(ret==WID_DBUS_ERROR)
				vty_out(vty,"operation fail! .\n");
			else if(ret==SET_MAX_STANUM_SMALLER_THAN_CURRENT_STANUM)
				vty_out(vty,"set max_stanum smaller than current stanum");
			else if(ret == Wlan_IF_NOT_BE_BINDED)     //fengwenchao add 20110512
				vty_out(vty,"wlan is not binded radio .\n");
			else if(ret == BSS_NOT_EXIST)      //fengwenchao add 20110512
				vty_out(vty,"bss is not exist .\n");
		}
	else if(TYPE==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set bss %s max sta num %s successfully .\n",id,argv[0],argv[1]);
					if((count != 0)&&(TYPE == 1)&&(RadioList_Head!=NULL)&&(failnum !=0))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<failnum; i++)
								{
									vty_out(vty,"%d ",RadioList_Head[i].RadioId);					
								}
							vty_out(vty," failed.\n");
							if(RadioList_Head != NULL)
								{
									free(RadioList_Head);
									RadioList_Head =NULL;
								}
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");		
		}	
	return CMD_SUCCESS;

}

#else
DEFUN(set_radio_bss_max_num_cmd_func,
	  set_radio_bss_max_num_cmd,
	  "set bss wlan WLANID max_sta_num NUM",             //fengwenchao change BSSINDEX to wlan WLANID  20110512
	  "set \n"
	  "based bss\n"
	  "local bss index\n"
	  "max station number\n"
	  "<0~32767>\n"                  //fengwenchao modify 20110415
	  "max station number\n"
	  "the number value\n"
	 )
{
	int ret=0;
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int radio_id=0;


	//unsigned int bss_index=0,max_sta_num=0;   //fengwenchao coment 20110512
	int max_sta_num = 0;   //fengwenchao add 20110512
	unsigned int type=1;
	unsigned int bssid;
	int stanum=0;  //fengwenchao change unsigned int to int 20110512
	unsigned int wlanid = 0;     //fengwenchao add 20110512
	unsigned char wlan_wid = 0; //fengwenchao add 20110512
	//radio_id = (int)vty->index;

	/*bss_index= atoi(argv[0]);*/
	ret = parse_int_ID((char *)argv[0],&wlanid);     //fengwenchao modify 20110512
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}
	/*max_sta_num= atoi(argv[1]);*/

	ret = parse_int_ID((char *)argv[1],&max_sta_num);

	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[1]);
		return CMD_SUCCESS;
	}

	if((wlanid < 1)||(wlanid > WLAN_NUM))                            //fengwenchao modify 20110512
	{
		vty_out(vty,"<error> wlanid should be 1 to %d\n",WLAN_NUM);
		return CMD_SUCCESS;
	}


	if((max_sta_num< 0)||(max_sta_num > 32767))    //fengwenchao modify 20110415
	{
		vty_out(vty,"<error> max station number should be greater than 0,and not cross 32767\n");
		return CMD_SUCCESS;
	}
	
	dbus_error_init(&err);

	/*compare max_sta_num  with  current sta num*/

	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_GET_STA_INFO);

	
	//bssid=radio_id*L_BSS_NUM+bss_index-1;   //fengwenchao comment 20110512
		
/*	query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_GET_STA_INFO);*/
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&type,
							DBUS_TYPE_UINT32,&wlanid,			//fengwenchao modify 20110512
							DBUS_TYPE_UINT32,&radio_id,   //fengwenchao modify 20110512
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
		//vty_out(vty,"<warning>bss maybe disable.\n");     //fengwenchao modify 20110512
		//return CMD_SUCCESS;       //fengwenchao comment 20110512
	}
	
	if(max_sta_num< stanum){
		vty_out(vty,"<error> %d sta(s) has accessed before you set max sta num %d  \n",stanum,max_sta_num);
		return CMD_SUCCESS;
	}

/*	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		radio_id = (int)vty->index_sub;
	}   */
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_BSS_MAX_STA);



	
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_SET_BSS_MAX_STA);*/
	wlan_wid = (unsigned char)wlanid;    //fengwenchao add 20110512

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_BYTE,&wlan_wid,            //fengwenchao modify 20110512
							 DBUS_TYPE_UINT32,&max_sta_num,
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
			vty_out(vty,"set bss %d max sta num %d successfully .\n",wlan_wid,max_sta_num);
		else if(ret==WLAN_ID_NOT_EXIST)      //fengwenchao modify 20110512
			vty_out(vty,"bss %d not exist .\n",wlan_wid);
		else if(ret==WID_DBUS_ERROR)
			vty_out(vty,"operation fail! .\n");
		else if(ret == Wlan_IF_NOT_BE_BINDED)     //fengwenchao add 20110512
			vty_out(vty,"wlan is not binded radio .\n");		
		else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
		{
			vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
		}
		/*else if(ret==ASD_BSS_VALUE_INVALIDE)
			//vty_out(vty,"more sta has accessed , before you set the max sta number\n",bss_index);*/
		else
		{
			vty_out(vty,"<error> other unknow error happend: %d\n",ret);
		}
	dbus_message_unref(reply);
	return CMD_SUCCESS;

}
#endif
#if _GROUP_POLICY
DEFUN(set_radio_bss_l3_policy_cmd_func,
	  set_radio_bss_l3_policy_cmd,
	  "set bss wlan WLANID (no|wlan|bss) interface",        //fengwenchao change <0-7> to wlan WLANID
	  "based bss\n"
	  "wlanid <1-129>\n"                 //fengwenchao modify 20110511
	  "bss L3 policy no wlan bss\n"
	  "L3 interface\n"
	 )
{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned int bsspolicy = 0;
	//unsigned int bss_id = 0;   //fengwenchao comment 20110511
	unsigned char wlanid = 0;      //fengwenchao add 20110511
	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 

	//radio_id = (int)vty->index;

	
	ret = parse_char_ID((char *)argv[0],&wlanid);   //fengwenchao modify 20110511
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}
	/*fengwenchao add 20110511*/
	if((wlanid < 1)||(wlanid > WLAN_NUM))
	{
		vty_out(vty,"<error> input parameter should be 1 to %d\n",WLAN_NUM);
		return CMD_SUCCESS;		
	}
	/*fengwenchao add end*/

	if (!strcmp(argv[1],"no"))/*bss use local mac mode*/
	{
		
    	bsspolicy = 0;
	}
	else if (!strcmp(argv[1],"wlan"))/*create a interface ,add it to the br*/
	{
		
    	bsspolicy = 1;
	}
	else if (!strcmp(argv[1],"bss"))/*use bss split mac mode*/
	{
		
    	bsspolicy = 2;
	}
	else
	{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}
	
	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    } 
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = set_radio_bss_l3_policy_cmd_bss_no_wlan_bss_interface(localid,index,dcli_dbus_connection,type,id,wlanid,bsspolicy,
	&count,&ret);     //fengwenchao modify 20110512

	if(type==0)
		{
			if(ret == 0)
				vty_out(vty,"set bss %s %s L3 interface successfully\n",argv[0],argv[1]);
			else if(ret == BSS_BE_ENABLE)
				vty_out(vty,"<error> BSS is enable, if you want to operate this, please disable it first\n");
			else if(ret == BSS_IF_NEED_DELETE)
				vty_out(vty,"<error> BSS l3 interface is exist,you should delete this interface first\n");
			else if(ret == BSS_CREATE_L3_INTERFACE_FAIL)
				vty_out(vty,"<error> BSS create l3 interface fail\n"); 
			else if(ret == BSS_DELETE_L3_INTERFACE_FAIL)
				vty_out(vty,"<error> BSS delete l3 interface fail\n");
			else if(ret == IF_POLICY_CONFLICT)
				vty_out(vty,"<error> WLAN policy is NO_INTERFACE,can not use this command\n");
			else if(ret == BSS_NOT_EXIST)
				vty_out(vty,"<error> BSS is not exist\n");
			else if(ret == UNKNOWN_ERROR)
				vty_out(vty,"<error> can not use this command\n");
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> RADIO is not exist\n");
			else if(ret == WTP_ID_NOT_EXIST)
				vty_out(vty,"<error> WTP is not exist\n");
			else if(ret == WTP_IS_NOT_BINDING_WLAN_ID)
				vty_out(vty,"<error> WTP is not binding wlan\n");
			else if(ret == WLAN_CREATE_BR_FAIL)
				vty_out(vty,"<error> WLAN br is not exist\n");
			else if(ret == BSS_L3_INTERFACE_ADD_BR_FAIL)
				vty_out(vty,"<error> add bss interface to wlan br fail\n");
			else if(ret == BSS_L3_INTERFACE_DEL_BR_FAIL)
				vty_out(vty,"<error> remove bss interface from wlan br fail\n");
			else if(ret == WLAN_ID_NOT_EXIST)           //fengwenchao add 20110511
				vty_out(vty,"<error> wlan id is not exist\n");
			else if(ret == Wlan_IF_NOT_BE_BINDED)              //fengwenchao add 20110511
				vty_out(vty,"<error> this wlan id is not binding radio\n");			
			else
				vty_out(vty,"<error>  %d\n",ret);
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set bss %s %s L3 interface successfully\n",id,argv[0],argv[1]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}	
	return CMD_SUCCESS;

}

#else
DEFUN(set_radio_bss_l3_policy_cmd_func,
	  set_radio_bss_l3_policy_cmd,
	  "set bss wlan WLANID (no|wlan|bss) interface",    //fengwenchao change <0-7> to wlan WLANID
	  "based bss\n"
	  "wlanid <1-129>\n"                //fengwenchao modify 20110511
	  "bss L3 policy no wlan bss\n"
	  "L3 interface\n"
	 )
{
	int ret=0;
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int radio_id=0;
	unsigned int bsspolicy = 0;


	//unsigned int bss_id = 0;   //fengwenchao comment 20110511
	unsigned char wlanid = 0;      //fengwenchao add 20110511

	//radio_id = (int)vty->index;

	
	ret = parse_char_ID((char *)argv[0],&wlanid);   //fengwenchao modify 20110511
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}
	/*fengwenchao add 20110511*/
	if((wlanid < 1)||(wlanid > WLAN_NUM))
	{
		vty_out(vty,"<error> input parameter should be 1 to %d\n",WLAN_NUM);
		return CMD_SUCCESS;		
	}
	/*fengwenchao add end*/

	if (!strcmp(argv[1],"no"))/*bss use local mac mode*/
	{
		
    	bsspolicy = 0;
	}
	else if (!strcmp(argv[1],"wlan"))/*create a interface ,add it to the br*/
	{
		
    	bsspolicy = 1;
	}
	else if (!strcmp(argv[1],"bss"))/*use bss split mac mode*/
	{
		
    	bsspolicy = 2;
	}
	else
	{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_BSS_L3_POLICY);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_SET_BSS_L3_POLICY);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_BYTE,&wlanid,       //fengwenchao modify 20110511
							 DBUS_TYPE_UINT32,&bsspolicy,
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
			vty_out(vty,"set bss %s %s L3 interface successfully\n",argv[0],argv[1]);
		else if(ret == BSS_BE_ENABLE)
			vty_out(vty,"<error> BSS is enable, if you want to operate this, please disable it first\n");
		else if(ret == BSS_IF_NEED_DELETE)
			vty_out(vty,"<error> BSS l3 interface is exist,you should delete this interface first\n");
		else if(ret == BSS_CREATE_L3_INTERFACE_FAIL)
			vty_out(vty,"<error> BSS create l3 interface fail\n"); 
		else if(ret == BSS_DELETE_L3_INTERFACE_FAIL)
			vty_out(vty,"<error> BSS delete l3 interface fail\n");
		else if(ret == IF_POLICY_CONFLICT)
			vty_out(vty,"<error> WLAN policy is NO_INTERFACE,can not use this command\n");
		else if(ret == BSS_NOT_EXIST)
			vty_out(vty,"<error> BSS is not exist\n");
		else if(ret == UNKNOWN_ERROR)
			vty_out(vty,"<error> can not use this command\n");
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> RADIO is not exist\n");
		else if(ret == WTP_ID_NOT_EXIST)
			vty_out(vty,"<error> WTP is not exist\n");
		else if(ret == WTP_IS_NOT_BINDING_WLAN_ID)
			vty_out(vty,"<error> WTP is not binding wlan\n");
		else if(ret == WLAN_CREATE_BR_FAIL)
			vty_out(vty,"<error> WLAN br is not exist\n");
		else if(ret == BSS_L3_INTERFACE_ADD_BR_FAIL)
			vty_out(vty,"<error> add bss interface to wlan br fail\n");
		else if(ret == BSS_L3_INTERFACE_DEL_BR_FAIL)
			vty_out(vty,"<error> remove bss interface from wlan br fail\n");
		else if(ret == WLAN_ID_NOT_EXIST)           //fengwenchao add 20110511
			vty_out(vty,"<error> wlan id is not exist\n");
		else if(ret == Wlan_IF_NOT_BE_BINDED)              //fengwenchao add 20110511
			vty_out(vty,"<error> this wlan id is not binding radio\n");
		else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
		{
			vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
		}
		else
			vty_out(vty,"<error>  %d\n",ret);
	
	return CMD_SUCCESS;

}
#endif
#if _GROUP_POLICY

DEFUN(set_radio_apply_wlan_cmd_func,
	  set_radio_apply_wlan_cmd,
	  "radio apply wlan ID",
	  "radio config \n"
	  "radio binding wlan\n"
	  "radio binding wlan\n"
	  "wlan id\n"
	 )
{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int g_radio_id = 0;
	unsigned int g_id = 0;
	unsigned int l_radio_id = 0;
	unsigned int type = 0;
	unsigned char wlanid = 0;

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 
	
	ret = parse_char_ID((char *)argv[0],&wlanid);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
       if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
       }
	   else{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
	   }
		return CMD_SUCCESS;
	}
	
	if((wlanid < 1)||(wlanid >(WLAN_NUM-1)))
	{
		vty_out(vty,"<error> input parameter should be 1 to %d \n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	if(vty->node == RADIO_NODE){
		index = 0;			
		g_radio_id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		g_radio_id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		g_radio_id = (unsigned)vty->index_sub;
    }else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		g_id = (unsigned int) vty->index_sub;
		l_radio_id = (unsigned int) vty->index_sub_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        g_id = (unsigned)vty->index_sub;
		l_radio_id = (unsigned int) vty->index_sub_sub;
		type= 1;
    } 
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	if (type == 0)
		RadioList_Head = set_radio_apply_wlan_cmd_radio_apply_wlan(localid,index,dcli_dbus_connection,type,g_radio_id,0, 0, wlanid,
	&count,&ret);
	else if (type == 1) {
		RadioList_Head = set_radio_apply_wlan_cmd_radio_apply_wlan(localid,index,dcli_dbus_connection,type,0, g_id, l_radio_id, wlanid,
	&count,&ret);
	} else {
		vty_out(vty, "error type %d\n", type);
	}

	if(type==0)
	{
		if(ret == 0)
			vty_out(vty,"radio %d-%d apply wlan %s successfully\n",g_radio_id/L_RADIO_NUM,g_radio_id%L_RADIO_NUM,argv[0]);
		else if(ret==RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio %d-%d not exist\n",g_radio_id/L_RADIO_NUM,g_radio_id%L_RADIO_NUM);
		else if(ret==WLAN_ID_NOT_EXIST)
			vty_out(vty,"<error> wlan %d not exist\n",wlanid);
		else if(ret==WTP_OVER_MAX_BSS_NUM)
			vty_out(vty,"<error> bss num is already %d\n",L_BSS_NUM);
		else if(ret==WTP_WLAN_BINDING_NOT_MATCH)
			vty_out(vty,"<error> wtp wlan binding interface not match\n");
		else if(ret==WTP_IF_NOT_BE_BINDED)
			vty_out(vty,"<error> wtp not bind interface\n");
		else if(ret==Wlan_IF_NOT_BE_BINDED)
			vty_out(vty,"<error> wlan not bind interface\n");
		else if(ret==WLAN_CREATE_L3_INTERFACE_FAIL)
			vty_out(vty,"<error> wlan crete wlan bridge fail\n");
		else if(ret==BSS_L3_INTERFACE_ADD_BR_FAIL)
			vty_out(vty,"<error> add bss if to wlan bridge fail\n");
		else if(ret == WTP_WEP_NUM_OVER)
			vty_out(vty,"<error> wtp over max wep wlan count 4 or wep index conflict\n");
		else if(ret == INTERFACE_BINDED_ALREADLY)
		{
			vty_out(vty,"<warning> radio has been binded this wlan already ,if you want use other ESSID,please unbind it first!\n");
		}
		else 
			vty_out(vty,"<error> radio %d-%d apply wlan %s fail\n",g_radio_id/L_RADIO_NUM,g_radio_id%L_RADIO_NUM,argv[0]);
	}
	else if(type==1)
	{
		if(ret == WID_DBUS_SUCCESS) {
			vty_out(vty,"group %d radio %d apply wlan %s successfully\n", g_id, l_radio_id, argv[0]);
			if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
			{
				vty_out(vty,"radio ");					
				for(i=0; i<count; i++)
				{
					if(Radio_Show_Node == NULL)
						Radio_Show_Node = RadioList_Head->RadioList_list;
					else 
						Radio_Show_Node = Radio_Show_Node->next;
					if(Radio_Show_Node == NULL)
						break;
					vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
				}
				vty_out(vty," failed.\n");
				dcli_free_RadioList(RadioList_Head);
			}
		} else if (ret == GROUP_ID_NOT_EXIST) {
	  		vty_out(vty,"<error> group id does not exist\n");
		} else if (ret == WLAN_ID_NOT_EXIST) {
			vty_out(vty, "<error> wlan %d is not exist\n", wlanid);
		}
		
	
	} else {
		vty_out(vty, "<error> unknown type %d\n", type);
	}
	return CMD_SUCCESS;

}

#else
DEFUN(set_radio_apply_wlan_cmd_func,
	  set_radio_apply_wlan_cmd,
	  "radio apply wlan ID",
	  "radio config \n"
	  "radio binding wlan\n"
	  "radio binding wlan\n"
	  "wlan id\n"
	 )
{
	int ret=0;
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	unsigned int radio_id=0;
	unsigned char wlanid = 0;
	//radio_id = (int)vty->index;
	
	ret = parse_char_ID((char *)argv[0],&wlanid);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
       if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
       }
	   else{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
	   }
		return CMD_SUCCESS;
	}
	
	if((wlanid < 1)||(wlanid >(WLAN_NUM-1)))
	{
		vty_out(vty,"<error> input parameter should be 1 to %d \n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_APPLY_WLAN);


/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_APPLY_WLAN);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_BYTE,&wlanid,
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
			vty_out(vty,"radio %d-%d apply wlan %s successfully\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0]);
		else if(ret==RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio %d-%d not exist\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM);
		else if(ret==WLAN_ID_NOT_EXIST)
			vty_out(vty,"<error> wlan %d not exist\n",wlanid);
		else if(ret==WTP_OVER_MAX_BSS_NUM)
			vty_out(vty,"<error> bss num is already %d\n",L_BSS_NUM);
		else if(ret==WTP_WLAN_BINDING_NOT_MATCH)
			vty_out(vty,"<error> wtp wlan binding interface not match\n");
		else if(ret==WTP_IF_NOT_BE_BINDED)
			vty_out(vty,"<error> wtp not bind interface\n");
		else if(ret==Wlan_IF_NOT_BE_BINDED)
			vty_out(vty,"<error> wlan not bind interface\n");
		else if(ret==WLAN_CREATE_L3_INTERFACE_FAIL)
			vty_out(vty,"<error> wlan crete wlan bridge fail\n");
		else if(ret==BSS_L3_INTERFACE_ADD_BR_FAIL)
			vty_out(vty,"<error> add bss if to wlan bridge fail\n");
		else if(ret == WTP_WEP_NUM_OVER)
			vty_out(vty,"<error> wtp over max wep wlan count 4 or wep index conflict\n");
		else if(ret == SECURITYINDEX_IS_SAME)   //fengwenchao add 20110112
			vty_out(vty,"<error> radio apply bingding securityindex is same with other\n");
		else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
		{
			vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
		}
		else if(ret == INTERFACE_BINDED_ALREADLY)
		{
			vty_out(vty,"<warning> radio has been binded this wlan already ,if you want use other ESSID,please unbind it first!\n");
		}
		else 
			vty_out(vty,"<error> radio %d-%d apply wlan %s fail\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0]);
		
	
	return CMD_SUCCESS;

}
#endif


DEFUN(set_radio_apply_wlan_base_essid_cmd_func,
	  set_radio_apply_wlan_base_essid_cmd,
	  "radio apply wlan ID essid .ESSID",
	  "radio config \n"
	  "radio binding wlan\n"
	  "radio binding wlan\n"
	  "wlan id\n"
	  "base\n"
	  "wlan ESSID\n"
	 )
{
	int ret=0,len=0;
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	unsigned int radio_id=0;
	unsigned char wlanid = 0;
	char *ESSID;
	//radio_id = (int)vty->index;
	
	ret = parse_char_ID((char *)argv[0],&wlanid);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
       if(ret == WID_ILLEGAL_INPUT)
	   {
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
       }
	   else
	   {
			vty_out(vty,"<error> input parameter %s error\n",argv[0]);
	   }
		return CMD_SUCCESS;
	}
	
	if((wlanid < 1)||(wlanid >(WLAN_NUM-1)))
	{
		vty_out(vty,"<error> input parameter should be 1 to %d \n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	ESSID= WID_parse_CMD_str(&argv[1],argc-1,NULL,0);
	if(ESSID== NULL)
	{		
		vty_out(vty,"UNKNOWN COMMAND\n");
		vty_out(vty,"COMMAND should be :create wlan 1 w1 aq w\n");
		return CMD_SUCCESS;
	}
	len = strlen(ESSID);
	if(len > 32)
	{		
		vty_out(vty,"<error> essid is too long,out of the limit of 32\n");
		if(ESSID)
		{
			free(ESSID);
			ESSID = NULL;
		}	
		return CMD_SUCCESS;
	}
	/*end*/
	if(-1 == ssid_illegal_character_check(ESSID,len))
	{
		vty_out(vty,"essid is null!or checkout the parameter len!\n");
		if(ESSID)
		{
			free(ESSID);
			ESSID = NULL;
		}
		return CMD_SUCCESS;
	}
	else if(-2==ssid_illegal_character_check(ESSID,len))
	{
		vty_out(vty,"illegal essid name!! ` \ \" & * ( ) not supported!\n");
		if(ESSID)
		{
			free(ESSID);
			ESSID = NULL;
		}
		return CMD_SUCCESS;
	}
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE)
	{
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE)
	{
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE)
	{
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_APPLY_WLAN_BASE_ESSID);


/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_APPLY_WLAN);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_BYTE,&wlanid,
							 DBUS_TYPE_STRING,&ESSID,
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
		if(ESSID)
		{
			free(ESSID);
			ESSID = NULL;
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_unref(reply);
		if(ret == 0)
			vty_out(vty,"radio %d-%d apply wlan %s successfully\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0]);
		else if(ret==RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio %d-%d not exist\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM);
		else if(ret==WLAN_ID_NOT_EXIST)
			vty_out(vty,"<error> wlan %d not exist\n",wlanid);
		else if(ret==WTP_OVER_MAX_BSS_NUM)
			vty_out(vty,"<error> bss num is already %d\n",L_BSS_NUM);
		else if(ret==WTP_WLAN_BINDING_NOT_MATCH)
			vty_out(vty,"<error> wtp wlan binding interface not match\n");
		else if(ret==WTP_IF_NOT_BE_BINDED)
			vty_out(vty,"<error> wtp not bind interface\n");
		else if(ret==Wlan_IF_NOT_BE_BINDED)
			vty_out(vty,"<error> wlan not bind interface\n");
		else if(ret==WLAN_CREATE_L3_INTERFACE_FAIL)
			vty_out(vty,"<error> wlan crete wlan bridge fail\n");
		else if(ret==BSS_L3_INTERFACE_ADD_BR_FAIL)
			vty_out(vty,"<error> add bss if to wlan bridge fail\n");
		else if(ret == WTP_WEP_NUM_OVER)
			vty_out(vty,"<error> wtp over max wep wlan count 4 or wep index conflict\n");
		else if(ret == SECURITYINDEX_IS_SAME)   //fengwenchao add 20110112
			vty_out(vty,"<error> radio apply bingding securityindex is same with other\n");
		else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
		{
			vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
		}
		else if(ret == INTERFACE_BINDED_ALREADLY)
		{
			vty_out(vty,"<warning> radio has been binded this wlan already ,if you want use other ESSID,please unbind it first!\n");
		}
		else 
			vty_out(vty,"<error> radio %d-%d apply wlan %s fail\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0]);
		
	free(ESSID);
	return CMD_SUCCESS;

}

#if _GROUP_POLICY
DEFUN(set_radio_apply_qos_cmd_func,
	  set_radio_apply_qos_cmd,
	  "radio apply qos ID",
	  "radio config \n"
	  "radio binding qos\n"
	  "radio binding qos\n"
	  "qos id <1-15>\n"
	 )
{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned int qosid = 0;
	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 

	
	ret = parse_int_ID((char *)argv[0],&qosid);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}
	
	if((qosid < 1)||(qosid >15))
	{
		vty_out(vty,"<error> input parameter should be 1 to %d \n",QOS_NUM-1);
		return CMD_SUCCESS;
	}

	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    } 
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = set_radio_apply_qos_cmd_radio_apply_qos(localid,index,dcli_dbus_connection,type,id,qosid,
	&count,&ret); 

	if(type==0)
		{
			if(ret == 0)
				vty_out(vty,"radio %d-%d apply qos %s successfully\n",id/L_RADIO_NUM,id%L_RADIO_NUM,argv[0]);
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio %d-%d not exist\n",id/L_RADIO_NUM,id%L_RADIO_NUM);
			else if(ret == WID_QOS_NOT_EXIST)
				vty_out(vty,"<error> qos %d not exist\n",qosid);
			else 
				vty_out(vty,"<error> radio %d-%d apply qos %s fail\n",id/L_RADIO_NUM,id%L_RADIO_NUM,argv[0]);
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d radio apply qos %s successfully\n",id,argv[0]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}
	
	return CMD_SUCCESS;

}

#else
DEFUN(set_radio_apply_qos_cmd_func,
	  set_radio_apply_qos_cmd,
	  "radio apply qos ID",
	  "radio config \n"
	  "radio binding qos\n"
	  "radio binding qos\n"
	  "qos id <1-15>\n"
	 )
{
	int ret=0;
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	unsigned int radio_id=0;
	int qosid = 0;
	//radio_id = (int)vty->index;
	
	ret = parse_int_ID((char *)argv[0],&qosid);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}
	
	if((qosid < 1)||(qosid >15))
	{
		vty_out(vty,"<error> input parameter should be 1 to %d \n",QOS_NUM-1);
		return CMD_SUCCESS;
	}
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_APPLY_QOS);


/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_APPLY_QOS);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_UINT32,&qosid,
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
			vty_out(vty,"radio %d-%d apply qos %s successfully\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0]);
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio %d-%d not exist\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM);
		else if(ret == WID_QOS_NOT_EXIST)
			vty_out(vty,"<error> qos %d not exist\n",qosid);
		else 
			vty_out(vty,"<error> radio %d-%d apply qos %s fail\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0]);
		
	
	return CMD_SUCCESS;

}
#endif
#if _GROUP_POLICY
DEFUN(set_radio_delete_qos_cmd_func,
	  set_radio_delete_qos_cmd,
	  "radio delete qos ID",
	  "radio config \n"
	  "radio release binding qos\n"
	  "radio release binding qos\n"
	  "qos id <1-15>\n"
	 )
{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned int qosid = 0;
	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 
	
	ret = parse_int_ID((char *)argv[0],&qosid);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}
	
	if((qosid < 1)||(qosid >15))
	{
		vty_out(vty,"<error> input parameter should be 1 to %d \n",QOS_NUM-1);
		return CMD_SUCCESS;
	}

	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    } 
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = set_radio_delete_qos_cmd_radio_delete_qos(localid,index,dcli_dbus_connection,type,id,qosid,
	&count,&ret);

	if(type==0)
		{
			if(ret == 0)
				vty_out(vty,"radio %d-%d delete qos %s successfully\n",id/L_RADIO_NUM,id%L_RADIO_NUM,argv[0]);
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio %d-%d not exist\n",id/L_RADIO_NUM,id%L_RADIO_NUM);
			else if(ret == WID_QOS_NOT_EXIST)
				vty_out(vty,"<error> qos %d not exist\n",qosid);
			else if(ret == WID_QOS_RADIO_SHOULD_BE_DISABLE)
				vty_out(vty,"<error> radio %d-%d is enable,please disable it first\n",id/L_RADIO_NUM,id%L_RADIO_NUM);
			else 
				vty_out(vty,"<error> radio %d-%d delete qos %s fail\n",id/L_RADIO_NUM,id%L_RADIO_NUM,argv[0]);
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d radio delete qos %s successfully\n",id,argv[0]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}
	return CMD_SUCCESS;

}

#else
DEFUN(set_radio_delete_qos_cmd_func,
	  set_radio_delete_qos_cmd,
	  "radio delete qos ID",
	  "radio config \n"
	  "radio release binding qos\n"
	  "radio release binding qos\n"
	  "qos id <1-15>\n"
	 )
{
	int ret=0;
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	unsigned int radio_id=0;
	int qosid = 0;
	//radio_id = (int)vty->index;
	
	ret = parse_int_ID((char *)argv[0],&qosid);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}
	
	if((qosid < 1)||(qosid >15))
	{
		vty_out(vty,"<error> input parameter should be 1 to %d \n",QOS_NUM-1);
		return CMD_SUCCESS;
	}
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_DELETE_QOS);


/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_DELETE_QOS);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_UINT32,&qosid,
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
			vty_out(vty,"radio %d-%d delete qos %s successfully\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0]);
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio %d-%d not exist\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM);
		else if(ret == WID_QOS_NOT_EXIST)
			vty_out(vty,"<error> qos %d not exist\n",qosid);
		else if(ret == WID_QOS_RADIO_SHOULD_BE_DISABLE)
			vty_out(vty,"<error> radio %d-%d is enable,please disable it first\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM);
		else 
			vty_out(vty,"<error> radio %d-%d delete qos %s fail\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0]);
		
	
	return CMD_SUCCESS;

}
#endif
#if _GROUP_POLICY
DEFUN(set_radio_bss_max_throughput_func,
	  set_radio_bss_max_throughput_cmd,
	  "set bss wlan WLANID max throughput THROUGHPUT",/*sz change 25 to 30 at 04/30, for ap request*/       //fengwenchao change <1-30> to THROUGHPUT   change BSSINDEX to wlan WLANID
	  "bss configuration\n"
	  "based bss\n"
	  "wlanid<1-129>\n"          //fengwenchao modify 20110512
	  "bss max throughput\n"
	  "bss max throughput\n"
	  "bss max throughput <1-30>\n"
	 )
{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0; 
	unsigned char wlanid = 0;
	unsigned int throughput = 0;
	//unsigned int bss_id = 0;   //fengwenchao comment 20110512
	//unsigned char wlanid = 0; //fengwenchao add 20110512	
	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 
	
	ret = parse_char_ID((char *)argv[0],&wlanid);       //fengwenchao modify 20110512
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}
	/*fengwenchao add 20110512*/
	if((wlanid < 1)||(wlanid > WLAN_NUM))
	{
		vty_out(vty,"<error> input parameter should be 1 to %d\n",WLAN_NUM);
		return CMD_SUCCESS;		
	}
	/*fengwenchao add end*/	
	throughput = atoi(argv[1]);
	/*fengwenchao add 20110504*/
	if((throughput < 1)||(throughput > 30))
	{
		vty_out(vty,"<error> input parameter should be 1-30\n");
		return CMD_SUCCESS;	
	}
	/*fengwenchao add end*/
	

	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    } 
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = set_radio_bss_max_throughput_cmd_set_bss_max_throughput(localid,index,dcli_dbus_connection,type,id,wlanid,throughput,   //fengwenchao modify 20110512
	&count,&ret);

	if(type==0)
		{
			if(ret == 0)
				vty_out(vty,"set bss %s max throughput %s successfully\n",argv[0],argv[1]);
			else if(ret == BSS_NOT_EXIST)
				vty_out(vty,"<error> BSS is not exist\n");
			else if(ret == WTP_ID_NOT_EXIST)
				vty_out(vty,"<error> WTP is not exist\n");
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> RADIO is not exist\n");
			else if(ret == WLAN_ID_NOT_EXIST)           //fengwenchao add 20110512
				vty_out(vty,"<error> wlan id is not exist\n");			
			else
				vty_out(vty,"<error>  %d\n",ret);
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set bss %s max throughput %s successfully\n",id,argv[0],argv[1]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}
	
	return CMD_SUCCESS;

}

#else
DEFUN(set_radio_bss_max_throughput_func,
	  set_radio_bss_max_throughput_cmd,
	  "set bss wlan WLANID max throughput THROUGHPUT",/*sz change 25 to 30 at 04/30, for ap request*/      //fengwenchao change <1-30> to THROUGHPUT   change BSSINDEX to wlanWLANID
	  "bss configuration\n"
	  "based bss\n"
	  "wlanid<1-129>\n"       //fengwenchao modify 20110512
	  "bss max throughput\n"
	  "bss max throughput\n"
	  "bss max throughput <1-30>\n"
	 )
{
	int ret=0;
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int radio_id=0;
	unsigned int throughput = 0;
	//unsigned int bss_id = 0;   //fengwenchao comment 20110512
	unsigned char wlanid = 0; //fengwenchao add 20110512
	//radio_id = (int)vty->index;
	//bss_id = atoi(argv[0]);
	
	ret = parse_char_ID((char *)argv[0],&wlanid);       //fengwenchao modify 20110512
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}
	/*fengwenchao add 20110512*/
	if((wlanid < 1)||(wlanid > WLAN_NUM))
	{
		vty_out(vty,"<error> input parameter should be 1 to %d\n",WLAN_NUM);
		return CMD_SUCCESS;		
	}
	/*fengwenchao add end*/
	throughput = atoi(argv[1]);
	/*printf("input radioid:%d bssid:%d throuput:%d\n",radio_id,bss_id,throughput);*/
	/*fengwenchao add 20110504*/
	if((throughput < 1)||(throughput > 30))
	{
		vty_out(vty,"<error> input parameter should be 1-30\n");
		return CMD_SUCCESS;	
	}
	/*fengwenchao add end*/	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_BSS_MAX_THROUGHPUT);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_SET_BSS_MAX_THROUGHPUT);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_BYTE,&wlanid,         //fengwenchao modify 20110512
							 DBUS_TYPE_UINT32,&throughput,
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
			vty_out(vty,"set bss %s max throughput %s successfully\n",argv[0],argv[1]);
		else if(ret == BSS_NOT_EXIST)
			vty_out(vty,"<error> BSS is not exist\n");
		else if(ret == WTP_ID_NOT_EXIST)
			vty_out(vty,"<error> WTP is not exist\n");
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> RADIO is not exist\n");
		else if(ret == WLAN_ID_NOT_EXIST)           //fengwenchao add 20110512
			vty_out(vty,"<error> wlan id is not exist\n");		
		else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
		{
			vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
		}
		else
			vty_out(vty,"<error>  %d\n",ret);
	
	return CMD_SUCCESS;

}
#endif
DEFUN(show_radio_bss_max_throughput_func,
	  show_radio_bss_max_throughput_cmd,
	  "show radio max throughput",
	  SHOW_STR
	  "radio infomation\n"
	  "radio bss max throughput\n"
	  "radio bss max throughput\n"
	 )
{
	int ret=0;
/*	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;*/
	WID_BSS *BSS[L_BSS_NUM];
	unsigned int radio_id = 0;
	int count = 0;
	int i = 0;
	unsigned char radio_throughput;
	/*unsigned int id[8];
	//unsigned char throughput[8];
	//memset(id,0,8);
	//memset(throughput,0,8);*/
	
	//radio_id = (int)vty->index;
	DCLI_RADIO_API_GROUP_ONE *RADIOINFO = NULL;
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#if 0	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SHOW_BSS_MAX_THROUGHPUT);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_SHOW_BSS_MAX_THROUGHPUT);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
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
	{
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&radio_throughput);
	
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&count);

		for(i=0;i<count;i++)
		{
			BSS[i]= (WID_BSS*)malloc(sizeof(WID_BSS));
			BSS[i]->BSSID = (unsigned char *)malloc(6);
			memset(BSS[i]->BSSID,0,6);
				
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&BSS[i]->BSSIndex);/*global bssindex, %32 = local bss index*/
			
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&BSS[i]->band_width);

			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(BSS[i]->BSSID[0]));

			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(BSS[i]->BSSID[1]));

			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(BSS[i]->BSSID[2]));

			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(BSS[i]->BSSID[3]));

			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(BSS[i]->BSSID[4]));

			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(BSS[i]->BSSID[5]));

			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&BSS[i]->bss_max_allowed_sta_num);
			
		}
	}			
	dbus_message_unref(reply);
#endif
		RADIOINFO = dcli_radio_show_api_group_one(
			index,
			radio_id,/*"show radio max throughput"*/
			&localid,
			&ret,
			0,
			//RADIOINFO,
			dcli_dbus_connection,
			WID_DBUS_RADIO_METHOD_SHOW_BSS_MAX_THROUGHPUT
			);	
		if(ret == -1){
			cli_syslog_info("<error> failed get reply.\n");
		}
		if(ret == 0){
			if(RADIOINFO->bss_num_int== 0)
			{
				vty_out(vty,"radioid	%d-%d\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM);
				vty_out(vty,"radio throughput:	%d\n",RADIOINFO->WTP[0]->WTP_Radio[0]->bandwidth);
				vty_out(vty,"no bss exist\n");
			}
			else
			{
				vty_out(vty,"radio %d-%d BSS throughput summary\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM);
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"radioid	%d-%d\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM);
				vty_out(vty,"radio throughput:	%d\n",RADIOINFO->WTP[0]->WTP_Radio[0]->bandwidth);
				for (i = 0; i < RADIOINFO->bss_num_int; i++) 
				{
					vty_out(vty,"bss index:	%d\n",RADIOINFO->WTP[0]->WTP_Radio[0]->BSS[i]->BSSIndex);
					vty_out(vty,"bss id:	%02X:%02X:%02X:%02X:%02X:%02X\n",RADIOINFO->WTP[0]->WTP_Radio[0]->BSS[i]->BSSID[0]
																			,RADIOINFO->WTP[0]->WTP_Radio[0]->BSS[i]->BSSID[1]
																			,RADIOINFO->WTP[0]->WTP_Radio[0]->BSS[i]->BSSID[2]
																			,RADIOINFO->WTP[0]->WTP_Radio[0]->BSS[i]->BSSID[3]
																			,RADIOINFO->WTP[0]->WTP_Radio[0]->BSS[i]->BSSID[4]
																			,RADIOINFO->WTP[0]->WTP_Radio[0]->BSS[i]->BSSID[5]);
					vty_out(vty,"throughput: %d\n",RADIOINFO->WTP[0]->WTP_Radio[0]->BSS[i]->band_width);
				/*	if(RADIOINFO->WTP[0]->WTP_Radio[0]->BSS[i]->BSSID)
					{
						free(RADIOINFO->WTP[0]->WTP_Radio[0]->BSS[i]->BSSID);
						RADIOINFO->WTP[0]->WTP_Radio[0]->BSS[i]->BSSID = NULL;
					}
					if(RADIOINFO->WTP[0]->WTP_Radio[0]->BSS[i])
					{
						free(RADIOINFO->WTP[0]->WTP_Radio[0]->BSS[i]);
						RADIOINFO->WTP[0]->WTP_Radio[0]->BSS[i] = NULL;
					}
					CW_FREE_OBJECT(RADIOINFO->WTP[0]->WTP_Radio[0]);
					CW_FREE_OBJECT(RADIOINFO->WTP[0]);
					CW_FREE_OBJECT(RADIOINFO->WTP);*/
				}
				vty_out(vty,"==============================================================================\n");
			
			}
			dcli_radio_free_fun(WID_DBUS_RADIO_METHOD_SHOW_BSS_MAX_THROUGHPUT,RADIOINFO);
		}
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> RADIO is not exist\n");
		else if(ret == WTP_ID_NOT_EXIST)
			vty_out(vty,"<error> WTP is not exist\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	
	return CMD_SUCCESS;

}
#if _GROUP_POLICY
DEFUN(set_radio_delete_wlan_cmd_func,
	  set_radio_delete_wlan_cmd,
	  "radio delete wlan ID",
	  "radio config \n"
	  "radio delete wlan\n"
	  "radio delete wlan\n"
	  "wlan id \n"
	 )
{

	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int g_radio_id = 0;
	unsigned int g_id;
	unsigned int l_radio_id;
	unsigned int type = 0; 
	unsigned char wlanid = 0;
	
	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 
	
	ret = parse_char_ID((char *)argv[0],&wlanid);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
       if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
       }
	   else{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
	   }
		return CMD_SUCCESS;
	}
	
	if((wlanid < 1)||(wlanid >(WLAN_NUM-1)))
	{
		vty_out(vty,"<error> input parameter should be 1 to %d \n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	if(vty->node == RADIO_NODE){
		index = 0;			
		g_radio_id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		g_radio_id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		g_radio_id = (unsigned)vty->index_sub;
    }else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		g_id = (unsigned)vty->index_sub;
		l_radio_id = (unsigned int)vty->index_sub_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        g_id = (unsigned)vty->index_sub;
		l_radio_id = (unsigned int)vty->index_sub_sub;
		type= 1;
    } 
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	if (type == 0)
		RadioList_Head = set_radio_delete_wlan_cmd_radio_delete_wlan(localid,index,dcli_dbus_connection,type, g_radio_id, 0, 0, wlanid,
		&count,&ret);
	else if (type == 1)
		RadioList_Head = set_radio_delete_wlan_cmd_radio_delete_wlan(localid,index,dcli_dbus_connection,type,0, g_id, l_radio_id, wlanid,
		&count,&ret);
	else 
		vty_out(vty, "unknown type\n");
	
	if(type==0)
	{
		if(ret == 0)
			vty_out(vty,"radio %d-%d delete wlan %s successfully\n",g_radio_id/L_RADIO_NUM,g_radio_id%L_RADIO_NUM,argv[0]);
		else if(ret==RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio %d-%d not exist\n",g_radio_id/L_RADIO_NUM,g_radio_id%L_RADIO_NUM);
		else if(ret==WLAN_ID_NOT_EXIST)
			vty_out(vty,"<error> wlan %d not exist\n",wlanid);
		else if (ret == INTERFACE_BINDED_OTHER_ESSID)
		{
			vty_out(vty,"<error> radio interface is binded to this wlan used other ESSID\n");
		}
		else if (ret == BSS_BE_ENABLE)
		{
			vty_out(vty,"<error> please disable wlan service first !\n");
		}
		else if (ret == RADIO_IN_EBR)
		{
			vty_out(vty,"<error> please delete radio interface from ebr first !\n");
		}
		else if(ret == INTERFACE_NOT_BE_BINDED)
			vty_out(vty,"<error>radio not binding wlan\n");
		else 
			vty_out(vty,"<error> radio %d-%d delete wlan %s fail\n",g_radio_id/L_RADIO_NUM,g_radio_id%L_RADIO_NUM,argv[0]);
	}
	else if(type==1)
	{
		if(ret == 0)
		{
			vty_out(vty,"group %d radio %d delete wlan %s successfully\n",g_id, l_radio_id, argv[0]);
			if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
			{
				vty_out(vty,"radio ");					
				for(i=0; i<count; i++)
				{
					if(Radio_Show_Node == NULL)
						Radio_Show_Node = RadioList_Head->RadioList_list;
					else 
						Radio_Show_Node = Radio_Show_Node->next;
					if(Radio_Show_Node == NULL)
						break;
					vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
				}
				vty_out(vty," failed.\n");
				dcli_free_RadioList(RadioList_Head);
			}
		} else if (ret == GROUP_ID_NOT_EXIST) {
			vty_out(vty,"<error> group id does not exist\n");
		} else if (ret == WLAN_ID_NOT_EXIST) {
			vty_out(vty, "<error> wlan %d is not exist\n", wlanid);
		}
	
	} else {
		vty_out(vty, "<error> unknown type %d\n", type);
	}
	return CMD_SUCCESS;

}

#else
DEFUN(set_radio_delete_wlan_cmd_func,
	  set_radio_delete_wlan_cmd,
	  "radio delete wlan ID",
	  "radio config \n"
	  "radio delete wlan\n"
	  "radio delete wlan\n"
	  "wlan id \n"
	 )
{
	int ret=0;
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	unsigned int radio_id=0;
	unsigned char wlanid = 0;
	//radio_id = (int)vty->index;
	
	ret = parse_char_ID((char *)argv[0],&wlanid);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
       if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
       }
	   else{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
	   }
		return CMD_SUCCESS;
	}
	
	if((wlanid < 1)||(wlanid >(WLAN_NUM-1)))
	{
		vty_out(vty,"<error> input parameter should be 1 to %d \n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_DELETE_WLAN);


/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_DELETE_WLAN);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_BYTE,&wlanid,
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
			vty_out(vty,"radio %d-%d delete wlan %s successfully\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0]);
		else if(ret==RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio %d-%d not exist\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM);
		else if(ret==WLAN_ID_NOT_EXIST)
			vty_out(vty,"<error> wlan %d not exist\n",wlanid);
		else if(ret == RADIO_IN_EBR)
			vty_out(vty,"<error> radio interface is in ebr,please delete it from ebr first\n");
		else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
		{
			vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
		}
		else if (ret == INTERFACE_BINDED_OTHER_ESSID)
		{
			vty_out(vty,"<error> radio interface is binded to this wlan used other ESSID\n");
		}
		else if (ret == BSS_BE_ENABLE)
		{
			vty_out(vty,"<error> please disable wlan service first !\n");
		}
		else if (ret == RADIO_IN_EBR)
		{
			vty_out(vty,"<error> please delete radio interface from ebr first !\n");
		}
		else 
			vty_out(vty,"<error> radio %d-%d delete wlan %s fail\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0]);
		
	
	return CMD_SUCCESS;

}
#endif

DEFUN(set_radio_delete_wlan_base_essid_cmd_func,
	  set_radio_delete_wlan_base_essid_cmd,
	  "radio delete wlan ID essid .ESSID",
	  "radio config \n"
	  "radio delete wlan\n"
	  "radio delete wlan\n"
	  "wlan id \n"
	  "base\n"
	  "ESSID\n"
	 )
{
	int ret=0,len = 0;
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	unsigned int radio_id=0;
	unsigned char wlanid = 0;
	char *ESSID;
	//radio_id = (int)vty->index;
	
	ret = parse_char_ID((char *)argv[0],&wlanid);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
       if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
       }
	   else{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
	   }
		return CMD_SUCCESS;
	}
	
	if((wlanid < 1)||(wlanid >(WLAN_NUM-1)))
	{
		vty_out(vty,"<error> input parameter should be 1 to %d \n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	ESSID= WID_parse_CMD_str(&argv[1],argc-1,NULL,0);
	if(ESSID== NULL)
	{		
		vty_out(vty,"UNKNOWN COMMAND\n");
		vty_out(vty,"COMMAND should be :create wlan 1 w1 aq w\n");
		return CMD_SUCCESS;
	}
	len = strlen(ESSID);
	if(len > 32)
	{		
		vty_out(vty,"<error> essid is too long,out of the limit of 32\n");
		if(ESSID)
		{
			free(ESSID);
			ESSID = NULL;
		}	
		return CMD_SUCCESS;
	}
	/*end*/
	if(-1 == ssid_illegal_character_check(ESSID,len))
	{
		vty_out(vty,"essid is null!or checkout the parameter len!\n");
		if(ESSID)
		{
			free(ESSID);
			ESSID = NULL;
		}
		return CMD_SUCCESS;
	}
	else if(-2==ssid_illegal_character_check(ESSID,len))
	{
		vty_out(vty,"illegal essid name!! ` \ \" & * ( ) not supported!\n");
		if(ESSID)
		{
			free(ESSID);
			ESSID = NULL;
		}
		return CMD_SUCCESS;
	}
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_DELETE_WLAN_BASE_ESSID);


/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_DELETE_WLAN);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_BYTE,&wlanid,
							 DBUS_TYPE_STRING,&ESSID,
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
		if(ESSID)
		{
			free(ESSID);
			ESSID = NULL;
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_unref(reply);
		if(ret == 0)
			vty_out(vty,"radio %d-%d delete wlan %s successfully\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0]);
		else if(ret==RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio %d-%d not exist\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM);
		else if(ret==WLAN_ID_NOT_EXIST)
			vty_out(vty,"<error> wlan %d not exist\n",wlanid);
		else if(ret == RADIO_IN_EBR)
			vty_out(vty,"<error> radio interface is in ebr,please delete it from ebr first\n");
		else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
		{
			vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
		}
		else if (ret == INTERFACE_BINDED_OTHER_ESSID)
		{
			vty_out(vty,"<error> radio interface is binded to this wlan used other ESSID\n");
		}
		else if (ret == BSS_BE_ENABLE)
		{
			vty_out(vty,"<error> please disable wlan service first !\n");
		}
		else if (ret == RADIO_IN_EBR)
		{
			vty_out(vty,"<error> please delete radio interface from ebr first !\n");
		}
		else 
			vty_out(vty,"<error> radio %d-%d delete wlan %s fail\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0]);
		
	free(ESSID);
	ESSID = NULL;
	return CMD_SUCCESS;

}

#if _GROUP_POLICY
DEFUN(set_radio_enable_wlan_cmd_func,
	  set_radio_enable_wlan_cmd,
	  "radio enable wlan ID",
	  "radio config \n"
	  "radio enable wlan\n"
	  "radio enable wlan\n"
	  "wlan id \n"
	 )
{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0; 
	unsigned char wlanid = 0;
	
	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 
	
	ret = parse_char_ID((char *)argv[0],&wlanid);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
       if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
       }
	   else{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
	   }
		return CMD_SUCCESS;
	}
	
	if((wlanid < 1)||(wlanid >(WLAN_NUM-1)))
	{
		vty_out(vty,"<error> input parameter should be 1 to %d \n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    } 
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = set_radio_enable_wlan_cmd_radio_enable_wlan(localid,index,dcli_dbus_connection,type,id,wlanid,
	&count,&ret);

	if(type==0)
		{
			if(ret == 0)
				vty_out(vty,"radio %d-%d enable wlan %s successfully\n",id/L_RADIO_NUM,id%L_RADIO_NUM,argv[0]);
			else if(ret==RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio %d-%d not exist\n",id/L_RADIO_NUM,id%L_RADIO_NUM);
			else if(ret==WLAN_ID_NOT_EXIST)
				vty_out(vty,"<error> wlan %d not exist\n",wlanid);
			else if(ret == WTP_WEP_NUM_OVER)
				vty_out(vty,"<error> wtp over max wep wlan count 4 or wep index conflict\n");
			else if(ret == VALUE_IS_NONEED_TO_CHANGE)
				vty_out(vty,"<error> radio is already enable this wlan\n");
			else if(ret == WTP_WLAN_BINDING_NOT_MATCH)
				vty_out(vty,"<error> wtp binding interface not match wlan binding interface\n");
			else if(ret == WTP_IS_NOT_BINDING_WLAN_ID)
				vty_out(vty,"<error> radio is not binding this wlan\n");
			else if(ret == WLAN_BE_DISABLE)
				vty_out(vty,"<error> wlan is disable ,you should enable it first\n");
			else 
				vty_out(vty,"<error> radio %d-%d enable wlan %s fail\n",id/L_RADIO_NUM,id%L_RADIO_NUM,argv[0]);
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d radio enable wlan %s successfully\n",id,argv[0]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}	
	return CMD_SUCCESS;

}

#else
DEFUN(set_radio_enable_wlan_cmd_func,
	  set_radio_enable_wlan_cmd,
	  "radio enable wlan ID",
	  "radio config \n"
	  "radio enable wlan\n"
	  "radio enable wlan\n"
	  "wlan id \n"
	 )
{
	int ret=0;
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	unsigned int radio_id=0;
	unsigned char wlanid = 0;
	//radio_id = (int)vty->index;
	
	ret = parse_char_ID((char *)argv[0],&wlanid);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
       if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
       }
	   else{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
	   }
		return CMD_SUCCESS;
	}
	
	if((wlanid < 1)||(wlanid >(WLAN_NUM-1)))
	{
		vty_out(vty,"<error> input parameter should be 1 to %d \n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_ENABLE_WLAN);


/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_ENABLE_WLAN);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_BYTE,&wlanid,
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
			vty_out(vty,"radio %d-%d enable wlan %s successfully\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0]);
		else if(ret==RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio %d-%d not exist\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM);
		else if(ret==WLAN_ID_NOT_EXIST)
			vty_out(vty,"<error> wlan %d not exist\n",wlanid);
		else if(ret == WTP_WEP_NUM_OVER)
			vty_out(vty,"<error> wtp over max wep wlan count 4 or wep index conflict\n");
		else if(ret == VALUE_IS_NONEED_TO_CHANGE)
			vty_out(vty,"<error> radio is already enable this wlan\n");
		else if(ret == WTP_WLAN_BINDING_NOT_MATCH)
			vty_out(vty,"<error> wtp binding interface not match wlan binding interface\n");
		else if(ret == WTP_IS_NOT_BINDING_WLAN_ID)
			vty_out(vty,"<error> radio is not binding this wlan\n");
		else if(ret == WLAN_BE_DISABLE)
			vty_out(vty,"<error> wlan is disable ,you should enable it first\n");
		else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
		{
			vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
		}
		else 
			vty_out(vty,"<error> radio %d-%d enable wlan %s fail\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0]);
		
	
	return CMD_SUCCESS;

}
#endif
#if _GROUP_POLICY
DEFUN(set_radio_disable_wlan_cmd_func,
	  set_radio_disable_wlan_cmd,
	  "radio disable wlan ID",
	  "radio config \n"
	  "radio disable wlan\n"
	  "radio disable wlan\n"
	  "wlan id \n"
	 )
{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0; 
	unsigned char wlanid = 0;
	
	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 	

	
	ret = parse_char_ID((char *)argv[0],&wlanid);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
       if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
       }
	   else{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
	   }
		return CMD_SUCCESS;
	}
	
	if((wlanid < 1)||(wlanid >(WLAN_NUM-1)))
	{
		vty_out(vty,"<error> input parameter should be 1 to %d \n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;id = (unsigned)vty->index_sub;
		id = (unsigned)vty->index_sub;
		type= 1;
    } 
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = set_radio_disable_wlan_cmd_radio_disable_wlan_ID(localid,index,dcli_dbus_connection,type,id,wlanid,
	&count,&ret);

	if(type==0)
		{
			if(ret == 0)
				vty_out(vty,"radio %d-%d disable wlan %s successfully\n",id/L_RADIO_NUM,id%L_RADIO_NUM,argv[0]);
			else if(ret==RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio %d-%d not exist\n",id/L_RADIO_NUM,id%L_RADIO_NUM);
			else if(ret==WLAN_ID_NOT_EXIST)
				vty_out(vty,"<error> wlan %d not exist\n",wlanid);
			else 
				vty_out(vty,"<error> radio %d-%d disable wlan %s fail\n",id/L_RADIO_NUM,id%L_RADIO_NUM,argv[0]);
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d radio disable wlan %s successfully\n",id,argv[0]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}	
	
	return CMD_SUCCESS;

}

#else
DEFUN(set_radio_disable_wlan_cmd_func,
	  set_radio_disable_wlan_cmd,
	  "radio disable wlan ID",
	  "radio config \n"
	  "radio disable wlan\n"
	  "radio disable wlan\n"
	  "wlan id \n"
	 )
{
	int ret=0;
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	unsigned int radio_id=0;
	unsigned char wlanid = 0;
	//radio_id = (int)vty->index;
	
	ret = parse_char_ID((char *)argv[0],&wlanid);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
       if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
       }
	   else{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
	   }
		return CMD_SUCCESS;
	}
	
	if((wlanid < 1)||(wlanid >(WLAN_NUM-1)))
	{
		vty_out(vty,"<error> input parameter should be 1 to %d \n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_DISABLE_WLAN);


/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_DISABLE_WLAN);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_BYTE,&wlanid,
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
			vty_out(vty,"radio %d-%d disable wlan %s successfully\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0]);
		else if(ret==RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio %d-%d not exist\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM);
		else if(ret==WLAN_ID_NOT_EXIST)
			vty_out(vty,"<error> wlan %d not exist\n",wlanid);
		else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
		{
			vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
		}
		else 
			vty_out(vty,"<error> radio %d-%d disable wlan %s fail\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0]);
		
	
	return CMD_SUCCESS;

}
#endif
#if _GROUP_POLICY

DEFUN(set_radio_default_config_cmd_func,
	  set_radio_default_config_cmd,
	  "recover default config",
	  "radio config recover\n"
	  "radio config recover\n"
	  "radio config recover\n"
	 )
{
	int i = 0;
	int ret = 0;
	int ret1 = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0; 
	unsigned char reserved = 0;
	
	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 	

	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    } 
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = set_radio_default_config_cmd_recover_default_config(localid,index,dcli_dbus_connection,type,id,reserved,
	&count,&ret);

	if(type==0)
		{
			if(ret == 0)
				vty_out(vty,"recover default config successfully\n");
			else 
				vty_out(vty,"<error> recover default config fail\n");
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d recover default config successfully\n",id);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}	
	return CMD_SUCCESS;

}

#else
DEFUN(set_radio_default_config_cmd_func,
	  set_radio_default_config_cmd,
	  "recover default config",
	  "radio config recover\n"
	  "radio config recover\n"
	  "radio config recover\n"
	 )
{
	int ret=0;
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	unsigned int radio_id=0;
	unsigned char reserved = 0;
	//radio_id = (int)vty->index;
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_REVOVER_DEFAULT_CONFIG);


/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_REVOVER_DEFAULT_CONFIG);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_BYTE,&reserved,
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
			vty_out(vty,"recover default config successfully\n");
		else 
			vty_out(vty,"<error> recover default config fail\n");
		
	
	return CMD_SUCCESS;

}
#endif

DEFUN(show_radio_channel_change_func,
	  show_radio_channel_change_cmd,
	  "show radio channel change info",
	  SHOW_STR
	  "radio infomation\n"
	  "radio radio channel change info\n"
	  "radio radio channel change info\n"
	  "radio radio channel change info\n"
	 )
{
	int ret=0;
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int radio_id = 0;
	unsigned short count = 0;
	unsigned short interval = 0;
	//radio_id = (int)vty->index;
	DCLI_RADIO_API_GROUP_ONE *RADIOINFO = NULL;
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#if 0	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SHOW_CHANNEL_CHANGE);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_SHOW_CHANNEL_CHANGE);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
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
	{
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&count);

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&interval);

	}			
	dbus_message_unref(reply);
#endif
		RADIOINFO = dcli_radio_show_api_group_one(
			index,
			radio_id,/*"show radio channel change info"*/
			&localid,
			&ret,
			0,
		//	RADIOINFO,
			dcli_dbus_connection,
			WID_DBUS_RADIO_METHOD_SHOW_CHANNEL_CHANGE
			);	
		if(ret == -1){
			cli_syslog_info("<error> failed get reply.\n");
		}
		if(ret == 0){
			vty_out(vty,"radio %d-%d channel change summary\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM);
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"channel change interval:	%d\n",RADIOINFO->interval);
			vty_out(vty,"channel change times:	%d\n",RADIOINFO->RADIO[0]->channelchangetime);
			vty_out(vty,"==============================================================================\n");
			dcli_radio_free_fun(WID_DBUS_RADIO_METHOD_SHOW_CHANNEL_CHANGE,RADIOINFO);
		}
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> RADIO is not exist\n");
		else if(ret == WTP_ID_NOT_EXIST)
			vty_out(vty,"<error> WTP is not exist\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	
	return CMD_SUCCESS;

}
#if _GROUP_POLICY
DEFUN(radio_apply_wlan_base_vlan_cmd_func,
		radio_apply_wlan_base_vlan_cmd,
		"radio apply wlan ID base vlan ID",
		"radio apply wlan id\n"
		"radio binding information\n" 
		"assign wlan id \n"
		"radio apply wlan base vlan\n"
		"radio apply wlan base vlan\n"
		"radio apply wlan base vlan ID <1-4094>\n"
		)
{
	int i = 0;
	int ret = 0;
	int ret1 = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0; 
	unsigned int vlan_id = 0;
	unsigned char wlan_id = 0;

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 		
	ret = parse_char_ID((char*)argv[0], &wlan_id);
	if (ret != WID_DBUS_SUCCESS)
	{	
       if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
       }
	   else{
			   vty_out(vty,"<error> input parameter %s error\n",argv[0]);
	   }
		
		return CMD_SUCCESS;
	}
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	ret = parse_int_ID((char*)argv[1], &vlan_id);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(vlan_id > VLANID_RANGE_MAX|| vlan_id == 0){
		vty_out(vty,"<error> vlan id should be 1 to %d\n",VLANID_RANGE_MAX);
		return CMD_SUCCESS;
	}
	
	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    } 
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = radio_apply_wlan_base_vlan_cmd_radio_apply_wlan_base_vlan(localid,index,dcli_dbus_connection,type,id,vlan_id,wlan_id,
	&count,&ret);

	if(type==0)
		{
			if(ret == 0)
				vty_out(vty,"radio %d-%d binding wlan %d base vlanid %d successfully.\n",id/L_RADIO_NUM,id%L_RADIO_NUM,wlan_id,vlan_id);
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio id does not exist\n");
			else if(ret == WTP_ID_BE_USED)
				vty_out(vty,"<error> wtp is in use, you should unused it first\n");
			else if(ret == WLAN_ID_NOT_EXIST)
				vty_out(vty,"<error> binding wlan does not exist \n");
			else if(ret == Wlan_IF_NOT_BE_BINDED)
				vty_out(vty,"<error> wlan does not bind interface\n");
			else if(ret == WTP_IF_NOT_BE_BINDED)
				vty_out(vty,"<error> wtp does not bind interface\n");
			else if(ret == WTP_WLAN_BINDING_NOT_MATCH)
				vty_out(vty,"<error> wlan and wtp bind interface did not match \n");
			else if(ret == WTP_CLEAR_BINDING_WLAN_SUCCESS)
				vty_out(vty,"clear wtp binding wlan list successfully\n");
			else if(ret == WLAN_BE_ENABLE)
				vty_out(vty,"<error> wlan is enable,you should disable it first\n");
			else if(ret == WTP_OVER_MAX_BSS_NUM)
				vty_out(vty,"<error> wtp over max bss count\n");	
			else if(ret == BSS_BE_ENABLE)
				vty_out(vty,"<error> bss is enable, you should disable it first\n");
			else if(ret == WTP_WEP_NUM_OVER)
				vty_out(vty,"<error> wtp over max wep wlan count 4 or wep index conflict\n");
			else
				vty_out(vty,"<error>  %d\n",ret);
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d radio binding wlan %d base vlanid %d successfully.\n",id,wlan_id,vlan_id);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}

	return CMD_SUCCESS;	

}

#else
DEFUN(radio_apply_wlan_base_vlan_cmd_func,
		radio_apply_wlan_base_vlan_cmd,
		"radio apply wlan ID base vlan ID",
		"radio apply wlan id\n"
		"radio binding information\n" 
		"assign wlan id \n"
		"radio apply wlan base vlan\n"
		"radio apply wlan base vlan\n"
		"radio apply wlan base vlan ID <1-4094>\n"
		)
{
	int ret;
	unsigned int radio_id;
	unsigned int vlan_id;
	unsigned char wlan_id;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	//radio_id = (unsigned int)vty->index;	

	
	ret = parse_char_ID((char*)argv[0], &wlan_id);
	if (ret != WID_DBUS_SUCCESS)
	{	
       if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
       }
	   else{
			   vty_out(vty,"<error> input parameter %s error\n",argv[0]);
	   }
		
		return CMD_SUCCESS;
	}
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	ret = parse_int_ID((char*)argv[1], &vlan_id);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(vlan_id > VLANID_RANGE_MAX|| vlan_id == 0){
		vty_out(vty,"<error> vlan id should be 1 to %d\n",VLANID_RANGE_MAX);
		return CMD_SUCCESS;
	}
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_RADIO_APPLY_WLANID_BASE_VLANID);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_RADIO_APPLY_WLANID_BASE_VLANID);*/

	dbus_error_init(&err);

	dbus_message_append_args(query,					
						DBUS_TYPE_UINT32,&radio_id,
						DBUS_TYPE_BYTE,&wlan_id,	
						DBUS_TYPE_UINT32,&vlan_id,
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
			vty_out(vty,"radio %d-%d binding wlan %d base vlanid %d successfully.\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,wlan_id,vlan_id);
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio id does not exist\n");
		else if(ret == WTP_ID_BE_USED)
			vty_out(vty,"<error> wtp is in use, you should unused it first\n");
		else if(ret == WLAN_ID_NOT_EXIST)
			vty_out(vty,"<error> binding wlan does not exist \n");
		else if(ret == Wlan_IF_NOT_BE_BINDED)
			vty_out(vty,"<error> wlan does not bind interface\n");
		else if(ret == WTP_IF_NOT_BE_BINDED)
			vty_out(vty,"<error> wtp does not bind interface\n");
		else if(ret == WTP_WLAN_BINDING_NOT_MATCH)
			vty_out(vty,"<error> wlan and wtp bind interface did not match \n");
		else if(ret == WTP_CLEAR_BINDING_WLAN_SUCCESS)
			vty_out(vty,"clear wtp binding wlan list successfully\n");
		else if(ret == WLAN_BE_ENABLE)
			vty_out(vty,"<error> wlan is enable,you should disable it first\n");
		else if(ret == WTP_OVER_MAX_BSS_NUM)
			vty_out(vty,"<error> wtp over max bss count\n");	
		else if(ret == BSS_BE_ENABLE)
			vty_out(vty,"<error> bss is enable, you should disable it first\n");
		else if(ret == WTP_WEP_NUM_OVER)
			vty_out(vty,"<error> wtp over max wep wlan count 4 or wep index conflict\n");
		else if(ret == SECURITYINDEX_IS_SAME)   //fengwenchao add 20110112
			vty_out(vty,"<error> radio apply bingding securityindex is same with other\n");
		else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
		{
			vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
		}
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);

	return CMD_SUCCESS;	

}
#endif

/*zhangcl added for REQUIREMENT-671*/
DEFUN(set_radio_cpe_channel_apply_wlan_base_vlan_cmd_func,
		set_radio_cpe_channel_apply_wlan_base_vlan_cmd,
		"radio cpe channel apply wlan ID base vlan ID",
		"radio apply wlan id\n"
		"cpe channel\n"
		"cpe channel\n"
		"radio binding information\n" 
		"assign wlan id \n"
		"radio apply wlan base vlan\n"
		"radio apply wlan base vlan\n"
		"radio apply wlan base vlan ID <1-4094>\n"
		)
{
	int ret;
	unsigned int radio_id;
	unsigned int vlan_id;
	unsigned char wlan_id;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	//radio_id = (unsigned int)vty->index;	

	
	ret = parse_char_ID((char*)argv[0], &wlan_id);
	if (ret != WID_DBUS_SUCCESS)
	{	
       if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
       }
	   else{
			   vty_out(vty,"<error> input parameter %s error\n",argv[0]);
	   }
		
		return CMD_SUCCESS;
	}
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	ret = parse_int_ID((char*)argv[1], &vlan_id);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(vlan_id > VLANID_RANGE_MAX|| vlan_id == 0){
		vty_out(vty,"<error> vlan id should be 1 to %d\n",VLANID_RANGE_MAX);
		return CMD_SUCCESS;
	}
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_RADIO_CPE_CHANNEL_APPLY_WLANID_BASE_VLANID);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_RADIO_APPLY_WLANID_BASE_VLANID);*/

	dbus_error_init(&err);

	dbus_message_append_args(query,					
						DBUS_TYPE_UINT32,&radio_id,
						DBUS_TYPE_BYTE,&wlan_id,	
						DBUS_TYPE_UINT32,&vlan_id,
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
			vty_out(vty,"radio %d-%d cpe channel binding wlan %d base vlanid %d successfully.\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,wlan_id,vlan_id);
		else if (ret == INTERFACE_NOT_BE_BINDED)
			vty_out(vty,"<error> radio should be binded to wlan first\n");
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio id does not exist\n");
		else if(ret == WTP_ID_BE_USED)
			vty_out(vty,"<error> wtp is in use, you should unused it first\n");
		else if(ret == WLAN_ID_NOT_EXIST)
			vty_out(vty,"<error> binding wlan does not exist \n");
		else if(ret == Wlan_IF_NOT_BE_BINDED)
			vty_out(vty,"<error> wlan does not bind interface\n");
		else if(ret == WTP_IF_NOT_BE_BINDED)
			vty_out(vty,"<error> wtp does not bind interface\n");
		else if(ret == WTP_WLAN_BINDING_NOT_MATCH)
			vty_out(vty,"<error> wlan and wtp bind interface did not match \n");
		else if(ret == WTP_CLEAR_BINDING_WLAN_SUCCESS)
			vty_out(vty,"clear wtp binding wlan list successfully\n");
		else if(ret == WLAN_BE_ENABLE)
			vty_out(vty,"<error> wlan is enable,you should disable it first\n");
		else if(ret == WTP_OVER_MAX_BSS_NUM)
			vty_out(vty,"<error> wtp over max bss count\n");	
		else if(ret == BSS_BE_ENABLE)
			vty_out(vty,"<error> bss is enable, you should disable it first\n");
		else if(ret == WTP_WEP_NUM_OVER)
			vty_out(vty,"<error> wtp over max wep wlan count 4 or wep index conflict\n");
		else if(ret == SECURITYINDEX_IS_SAME)   //fengwenchao add 20110112
			vty_out(vty,"<error> radio apply bingding securityindex is same with other\n");
		else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
		{
			vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
		}
		else if(ret == VALUE_OUT_OF_RANGE)
		{
			vty_out(vty,"<error> radio binding more than 8 wlan\n");
		}
		else if (ret == RADIO_ID_BE_USED)
		{
			vty_out(vty,"<error > radio cpe channel interface already exists !\n");
		}
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);

	return CMD_SUCCESS;	

}

//mahz add 2011.5.30
DEFUN(radio_apply_wlan_base_nas_port_id_cmd_func,
		radio_apply_wlan_base_nas_port_id_cmd,
		"radio apply wlan ID base nas_port_id NAS-PORT-ID",
		"radio apply wlan id\n"
		"radio binding information\n" 
		"assign wlan id \n"
		)
{
	int ret;
	int len = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char *nas_port_id = NULL;
	unsigned int radio_id;
	unsigned char wlan_id;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	ret = parse_char_ID((char*)argv[0], &wlan_id);
	if (ret != WID_DBUS_SUCCESS){	
       if(ret == WID_ILLEGAL_INPUT){
          	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
       }
	   else{
		   vty_out(vty,"<error> input parameter %s error\n",argv[0]);
	   }
		return CMD_SUCCESS;
	}
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	len = strlen(argv[1]);
	if(len > 32){		
		vty_out(vty,"<error> nas-port-id name is too long,out of the limit of 32\n");
		return CMD_SUCCESS;
	}
	
	nas_port_id = (char*)malloc(strlen(argv[1])+1);
	memset(nas_port_id, 0, strlen(argv[1])+1);
	memcpy(nas_port_id, argv[1], strlen(argv[1]));		

	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
	
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_RADIO_APPLY_WLANID_BASE_NAS_PORT_ID);

	dbus_error_init(&err);

	dbus_message_append_args(query,					
						DBUS_TYPE_UINT32,&radio_id,
						DBUS_TYPE_BYTE,&wlan_id,	
						DBUS_TYPE_STRING,&nas_port_id,
						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		if(nas_port_id){
			free(nas_port_id);
			nas_port_id = NULL;
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
		vty_out(vty,"radio %d-%d binding wlan %d base nas_port_id %s successfully.\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,wlan_id,nas_port_id);
	else if(ret == RADIO_ID_NOT_EXIST)
		vty_out(vty,"<error> radio id does not exist\n");
	else if(ret == WTP_ID_BE_USED)
		vty_out(vty,"<error> wtp is in use, you should unused it first\n");
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> binding wlan does not exist \n");
	else if(ret == Wlan_IF_NOT_BE_BINDED)
		vty_out(vty,"<error> wlan does not bind interface\n");
	else if(ret == WTP_IF_NOT_BE_BINDED)
		vty_out(vty,"<error> wtp does not bind interface\n");
	else if(ret == WTP_WLAN_BINDING_NOT_MATCH)
		vty_out(vty,"<error> wlan and wtp bind interface did not match \n");
	else if(ret == WTP_CLEAR_BINDING_WLAN_SUCCESS)
		vty_out(vty,"clear wtp binding wlan list successfully\n");
	else if(ret == WLAN_BE_ENABLE)
		vty_out(vty,"<error> wlan is enable,you should disable it first\n");
	else if(ret == WTP_OVER_MAX_BSS_NUM)
		vty_out(vty,"<error> wtp over max bss count\n");	
	else if(ret == BSS_BE_ENABLE)
		vty_out(vty,"<error> bss is enable, you should disable it first\n");
	else if(ret == WTP_WEP_NUM_OVER)
		vty_out(vty,"<error> wtp over max wep wlan count 4 or wep index conflict\n");
	else if(ret == SECURITYINDEX_IS_SAME)   //fengwenchao add 20110112
		vty_out(vty,"<error> radio apply bingding securityindex is same with other\n");
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

DEFUN(radio_apply_wlan_clean_nas_port_id_cmd_func,
		radio_apply_wlan_clean_nas_port_id_cmd,
		"radio apply wlan ID clean nas_port_id",
		CONFIG_STR
		"radio binding information\n" 
		"radio binding wlan information\n" 
		"assign wlan id \n"
		"radio apply wlan clean nas_port_id\n"
		"radio apply wlan clean nas_port_id\n"
		)
{
	int ret;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int radio_id;
	unsigned char wlan_id;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
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
	
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
	
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_RADIO_APPLY_WLANID_CLEAN_NAS_PORT_ID);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,					
						DBUS_TYPE_UINT32,&radio_id,
						DBUS_TYPE_BYTE,&wlan_id,	
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

		if(ret == 0)
			vty_out(vty,"radio %d-%d binding wlan %d clean nas_port_id successfully.\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,wlan_id);
		else if(ret == WTP_ID_NOT_EXIST)
			vty_out(vty,"<error> wtp id does not exist\n");
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio id does not exist\n");
		else if(ret == WLAN_ID_NOT_EXIST)
			vty_out(vty,"<error> binding wlan does not exist \n");
		else if(ret == WTP_WLAN_BINDING_NOT_MATCH)
			vty_out(vty,"<error> radio is not binding this wlan \n");
		else if(ret == BSS_BE_ENABLE)
			vty_out(vty,"<error> bss is enable, you should disable it first\n");
		else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
		{
			vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
		}
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);

	return CMD_SUCCESS;	

}
//weichao add
DEFUN(radio_apply_wlan_base_hotspot_cmd_func,
		radio_apply_wlan_base_hotspot_cmd,
		"radio apply wlan ID base hotspot  ID",
		"radio apply wlan id\n"
		"radio binding information\n" 
		"assign wlan id \n"
		"radio apply wlan base hotspot\n"
		"radio apply wlan base hotspot\n"
		"radio apply wlan base hotspot ID <1-4096>\n"
		)
{
	int ret;
	int localid = 1;
    	int slot_id = HostSlotId;
	unsigned int radio_id;
	unsigned int hotspot_id;
	unsigned char wlan_id;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	

	
	ret = parse_char_ID((char*)argv[0], &wlan_id);
	if (ret != WID_DBUS_SUCCESS)
	{	
       if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
       }
	   else{
			   vty_out(vty,"<error> input parameter %s error\n",argv[0]);
	   }
		
		return CMD_SUCCESS;
	}
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	ret = parse_int_ID((char*)argv[1], &hotspot_id);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(hotspot_id > HOTSPOT_ID|| hotspot_id == 0){
		vty_out(vty,"<error> hotspot id should be 1 to %d\n",HOTSPOT_ID);
		return CMD_SUCCESS;
	}
	
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
      		slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
	        index = vty->index;
	        localid = vty->local;
	        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
  	  }
	
  	DBusConnection *dcli_dbus_connection = NULL;
 	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_RADIO_APPLY_WLANID_BASE_HOTSPOT_ID);


	dbus_error_init(&err);

	dbus_message_append_args(query,					
						DBUS_TYPE_UINT32,&radio_id,
						DBUS_TYPE_BYTE,&wlan_id,	
						DBUS_TYPE_UINT32,&hotspot_id,
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

		if(ret == 0)
			vty_out(vty,"radio %d-%d binding wlan %d base hotspot %d successfully.\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,wlan_id,hotspot_id);
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio id does not exist\n");
		else if(ret == WTP_ID_BE_USED)
			vty_out(vty,"<error> wtp is in use, you should unused it first\n");
		else if(ret == WLAN_ID_NOT_EXIST)
			vty_out(vty,"<error> binding wlan does not exist \n");
		else if(ret == Wlan_IF_NOT_BE_BINDED)
			vty_out(vty,"<error> wlan does not bind interface\n");
		else if(ret == WTP_IF_NOT_BE_BINDED)
			vty_out(vty,"<error> wtp does not bind interface\n");
		else if(ret == WTP_WLAN_BINDING_NOT_MATCH)
			vty_out(vty,"<error> wlan and wtp bind interface did not match \n");
		else if(ret == WTP_CLEAR_BINDING_WLAN_SUCCESS)
			vty_out(vty,"clear wtp binding wlan list successfully\n");
		else if(ret == WLAN_BE_ENABLE)
			vty_out(vty,"<error> wlan is enable,you should disable it first\n");
		else if(ret == WTP_OVER_MAX_BSS_NUM)
			vty_out(vty,"<error> wtp over max bss count\n");	
		else if(ret == BSS_BE_ENABLE)
			vty_out(vty,"<error> bss is enable, you should disable it first\n");
		else if(ret == WTP_WEP_NUM_OVER)
			vty_out(vty,"<error> wtp over max wep wlan count 4 or wep index conflict\n");
		else if(ret == SECURITYINDEX_IS_SAME)   
			vty_out(vty,"<error> radio apply bingding securityindex is same with other\n");
		else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
		{
			vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
		}
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);

	return CMD_SUCCESS;	

}
DEFUN(radio_apply_wlan_clean_hotspot_cmd_func,
		radio_apply_wlan_clean_hotspot_cmd,
		"radio apply wlan ID clean hotspot_id",
		CONFIG_STR
		"radio binding information\n" 
		"radio binding wlan information\n" 
		"assign wlan id \n"
		"radio apply wlan clean hotspot_id\n"
		"radio apply wlan clean hotspot_id\n"
		)
{
	int ret;
	int localid = 1;
    	int slot_id = HostSlotId;
	unsigned int radio_id = 0;
	unsigned char wlan_id = 0;
	DBusMessage *query = NULL, *reply = NULL;	
	DBusMessageIter	 iter;
	DBusError err;
	
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
	
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
      		slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
	        index = vty->index;
	        localid = vty->local;
	        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
  	  }
	
  	DBusConnection *dcli_dbus_connection = NULL;
 	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_RADIO_APPLY_WLANID_CLEAN_HOTSPOT_ID);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,					
						DBUS_TYPE_UINT32,&radio_id,
						DBUS_TYPE_BYTE,&wlan_id,	
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

		if(ret == 0)
			vty_out(vty,"radio %d-%d binding wlan %d clean hotspot_id successfully.\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,wlan_id);
		else if(ret == WTP_ID_NOT_EXIST)
			vty_out(vty,"<error> wtp id does not exist\n");
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio id does not exist\n");
		else if(ret == WLAN_ID_NOT_EXIST)
			vty_out(vty,"<error> binding wlan does not exist \n");
		else if(ret == WTP_WLAN_BINDING_NOT_MATCH)
			vty_out(vty,"<error> radio is not binding this wlan \n");
		else if(ret == BSS_BE_ENABLE)
			vty_out(vty,"<error> bss is enable, you should disable it first\n");
		else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
		{
			vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
		}
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);

	return CMD_SUCCESS;	

}

//

/*
DEFUN(set_radio_bss_l3_interface_br_func,
	  set_radio_bss_l3_interface_br_cmd,
	  "(add|remove) bss <0-7> wlan br",
	  "add|remove bss to/from wlan br\n"
	  "bss operation\n"
	  "local bss index<0-3>\n"
	  "add|remove bss to/from wlan br\n"
	  "add|remove bss to/from wlan br\n"
	 )
{
	int ret=0;
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int radio_id=0;
	unsigned int bsspolicy = 0;


	unsigned int bss_id = 0;

	radio_id = (int)vty->index;

	if (!strcmp(argv[0],"add"))//add bss if to wlan br
	{
		
    	bsspolicy = 1;
	}
	else if (!strcmp(argv[0],"remove"))//remove bss if from br
	{
		
    	bsspolicy = 0;
	}
	else
	{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}
	
	ret = parse_int_ID((char *)argv[1],&bss_id);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[1]);
		return CMD_SUCCESS;
	}

	
	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_SET_BSS_L3_IF_WLAN_BR);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_UINT32,&bss_id,
							 DBUS_TYPE_UINT32,&bsspolicy,
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
			vty_out(vty,"set bss %s %s L3 interface successfully\n",argv[0],argv[1]);
		else if(ret == BSS_BE_ENABLE)
			vty_out(vty,"<error> BSS is enable, if you want to operate this, please disable it first\n");
		else if(ret == BSS_CREATE_L3_INTERFACE_FAIL)
			vty_out(vty,"<error> BSS create l3 interface fail\n"); 
		else if(ret == BSS_DELETE_L3_INTERFACE_FAIL)
			vty_out(vty,"<error> BSS delete l3 interface fail\n");
		else if(ret == IF_POLICY_CONFLICT)
			vty_out(vty,"<error> WLAN policy is NO_INTERFACE,can not use this command\n");
		else if(ret == BSS_NOT_EXIST)
			vty_out(vty,"<error> BSS is not exist\n");
		else if(ret == UNKNOWN_ERROR)
			vty_out(vty,"<error> can not use this command\n");
		else if(ret == WTP_ID_NOT_EXIST)
			vty_out(vty,"<error> WTP is not exist\n");
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> RADIO is not exist\n");
		else if(ret == WTP_IS_NOT_BINDING_WLAN_ID)
			vty_out(vty,"<error> WTP is not binding wlan\n");
		else if(ret == BSS_IF_NEED_CREATE)
			vty_out(vty,"<error> BSS if need to be created\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	
	return CMD_SUCCESS;

}
*/
#if _GROUP_POLICY
DEFUN(set_radio_max_throughout_func,
	  set_radio_max_throughout_cmd,
	  "set radio max throughout PARAMETER",
	  "wireless-control config\n"
	  "radio config\n"
	  "max throughout\n"
	  "max throughout\n"
	  "radio max throughout Mb/s \n"
	 )
{

	int i = 0;
	int ret = 0;
	int ret1 = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0; 
	unsigned char bandwidth = 0;

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 	
	
	ret1 = parse_char_ID((char*)argv[0], &bandwidth);
	if(ret1 != WID_DBUS_SUCCESS){
            if(ret1 == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> max throughout should be 1 to 108\n");
            }
			else{
			    vty_out(vty,"<error> unknown id format\n");
			}
			return CMD_SUCCESS;
	}
	if(bandwidth > 108 || bandwidth == 0){
		vty_out(vty,"<error> max throughout should be 1 to 108\n");
		return CMD_SUCCESS;
	}
	
	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    } 
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = set_radio_max_throughout_cmd_set_radio_max_throughout(localid,index,dcli_dbus_connection,type,id,bandwidth,
	&count,&ret);

	if(type==0)
		{
			if(ret == 0)
				{
					vty_out(vty,"set radioid max throughout %s successfully\n",argv[0]);
				}
			else if(ret == WTP_ID_NOT_EXIST)
					vty_out(vty,"<error> WTP id does not exist\n");
			else if(ret == RADIO_ID_NOT_EXIST)
					vty_out(vty,"<error> radio id does not exist\n");
			else
				{
					vty_out(vty,"<error>  %d\n",ret);
				}
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set radioid max throughout %s successfully\n",id,argv[0]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}
		
	return CMD_SUCCESS; 		
}

#else
DEFUN(set_radio_max_throughout_func,
	  set_radio_max_throughout_cmd,
	  "set radio max throughout PARAMETER",
	  "wireless-control config\n"
	  "radio config\n"
	  "max throughout\n"
	  "max throughout\n"
	  "radio max throughout Mb/s \n"
	 )
{
	int ret,ret1;

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	unsigned char bandwidth = 0;
	unsigned int radioid = 0;
	//radioid = (unsigned int)vty->index;
	
	ret1 = parse_char_ID((char*)argv[0], &bandwidth);
	if(ret1 != WID_DBUS_SUCCESS){
            if(ret1 == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> max throughout should be 1 to 108\n");
            }
			else{
			    vty_out(vty,"<error> unknown id format\n");
			}
			return CMD_SUCCESS;
	}
	if(bandwidth > 108 || bandwidth == 0){
		vty_out(vty,"<error> max throughout should be 1 to 108\n");
		return CMD_SUCCESS;
	}
	
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radioid = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		radioid = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radioid = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_MAX_THROUGHOUT);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_SET_MAX_THROUGHOUT);*/
	
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radioid,	
							 DBUS_TYPE_BYTE,&bandwidth,
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
		vty_out(vty,"set radioid max throughout %s successfully\n",argv[0]);
	}
	else if(ret == WTP_ID_NOT_EXIST)
		vty_out(vty,"<error> WTP id does not exist\n");
	else if(ret == RADIO_ID_NOT_EXIST)
		vty_out(vty,"<error> radio id does not exist\n");
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return CMD_SUCCESS; 		
}
#endif
DEFUN(show_radio_qos_cmd_func,
		 show_radio_qos_cmd,
		 "show radio qos list [remote] [local] [PARAMETER]",
		 SHOW_STR
		 "Display Radio qos Information\n"
		 "Display Radio qos Information\n"
		 "Display Radio qos Information\n"
		 "'remote' or 'local' hansi\n"
		 "'remote' or 'local' hansi\n"
		 "slotid-instid\n"
)
{
/*	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	*/
	
//	WID_WTP_RADIO *RADIO;	
//	RADIO = malloc(G_RADIO_NUM*(sizeof(WID_WTP_RADIO)));
	int ret,i=0;
	unsigned int num = 0;
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	DCLI_RADIO_API_GROUP_ONE *RADIOINFO = NULL;
	//CW_CREATE_OBJECT_ERR(RADIOINFO, DCLI_RADIO_API_GROUP_ONE, return NULL;);	
	//RADIOINFO->RADIO = NULL;

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
#if 0	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SHOW_RADIO_QOS);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_RADIO_METHOD_SHOW_RADIO_QOS);*/
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		if(RADIO)
		{
			free(RADIO);
			RADIO = NULL;
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
			
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(RADIO[i].Radio_G_ID));
		
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(RADIO[i].QOSID));
					
			dbus_message_iter_next(&iter_array);
		}
	}
	
	dbus_message_unref(reply);
#endif	
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
		RADIOINFO = dcli_radio_show_api_group_one(
			index,
			0,/*"show radio qos list"*/
			&localid,
			&ret,
			0,
			//RADIOINFO,
			dcli_dbus_connection,
			WID_DBUS_RADIO_METHOD_SHOW_RADIO_QOS
			);	
		
	    vty_out(vty,"Radio qos summary\n");
	    vty_out(vty,"==============================================================================\n");
		
		if(ret == -1){
			cli_syslog_info("<error> failed get reply.\n");
		}
		if(ret == 0){
			vty_out(vty,"RadioID	QOSID\n");
			for (i = 0; i <RADIOINFO->qos_num; i++)
			{
				vty_out(vty,"%-7d	%-5d\n",RADIOINFO->RADIO[i]->Radio_G_ID,RADIOINFO->RADIO[i]->QOSID);
			}
			vty_out(vty,"==============================================================================\n");
			//CW_FREE_OBJECT(RADIOINFO->RADIO);
			dcli_radio_free_fun(WID_DBUS_RADIO_METHOD_SHOW_RADIO_QOS,RADIOINFO);
		}
		else if(ret == RADIO_ID_NOT_EXIST)
		{
			vty_out(vty,"<error> there is no radio exist\n");
		}
		//CW_FREE_OBJECT(RADIOINFO);
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
				RADIOINFO = dcli_radio_show_api_group_one(
					profile,
					0,/*"show radio qos list"*/
					&localid,
					&ret,
					0,
					dcli_dbus_connection,
					WID_DBUS_RADIO_METHOD_SHOW_RADIO_QOS
					);	
				
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
			    vty_out(vty,"Radio qos summary\n");
			    vty_out(vty,"========================================================================\n");
				
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				if(ret == 0){
					vty_out(vty,"RadioID	QOSID\n");
					for (i = 0; i <RADIOINFO->qos_num; i++)
					{
						vty_out(vty,"%-7d	%-5d\n",RADIOINFO->RADIO[i]->Radio_G_ID,RADIOINFO->RADIO[i]->QOSID);
					}
					dcli_radio_free_fun(WID_DBUS_RADIO_METHOD_SHOW_RADIO_QOS,RADIOINFO);
				}
				else if(ret == RADIO_ID_NOT_EXIST)
				{
					vty_out(vty,"<error> there is no radio exist\n");
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
				RADIOINFO = dcli_radio_show_api_group_one(
					profile,
					0,/*"show radio qos list"*/
					&localid,
					&ret,
					0,
					dcli_dbus_connection,
					WID_DBUS_RADIO_METHOD_SHOW_RADIO_QOS
					);	
				
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
			    vty_out(vty,"Radio qos summary\n");
			    vty_out(vty,"========================================================================\n");
				
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				if(ret == 0){
					vty_out(vty,"RadioID	QOSID\n");
					for (i = 0; i <RADIOINFO->qos_num; i++)
					{
						vty_out(vty,"%-7d	%-5d\n",RADIOINFO->RADIO[i]->Radio_G_ID,RADIOINFO->RADIO[i]->QOSID);
					}
					dcli_radio_free_fun(WID_DBUS_RADIO_METHOD_SHOW_RADIO_QOS,RADIOINFO);
				}
				else if(ret == RADIO_ID_NOT_EXIST)
				{
					vty_out(vty,"<error> there is no radio exist\n");
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

DEFUN(show_radio_bss_cmd_func,
		 show_radio_bss_cmd,
		 "show radio bss list",
		 SHOW_STR
		 "Display Radio bss Information\n"
		 "Display Radio bss Information\n"
		 "Display Radio bss Information\n"
)
{
/*	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	*/
	
	WID_BSS *BSS[L_BSS_NUM];	
	int ret,i=0;
	unsigned int radioid = 0;
/*	unsigned char auto_channel = 0;
	unsigned char diversity = 0;
	unsigned char txantenna = 0;*/
	//radioid = (unsigned int)vty->index;
	unsigned int num = 0;
	char  dis[] = "disable";
	char  en[] = "enable";
	char if_policy[3][8] = {"NO_IF","WLAN_IF","BSS_IF"};
	char forward_policy[3][8] = {"UNKNOWN","BRIDGE","ROUTE"};
	
	char  capwap3[] = "CAPWAP802DOT3";
	char  capwap11[] = "CAPWAP802DOT11";
	char  ipip[] = "IPIP";

	char txantenna_policy[3][5] = {"AUTO","MAIN","VICE"};
	char *nas_id;
	DCLI_RADIO_API_GROUP_ONE *RADIOINFO = NULL;
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	if(vty->node == RADIO_NODE){
		index = 0;			
		radioid = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		radioid = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radioid = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#if 0	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SHOW_BSS_LIST);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_SHOW_BSS_LIST);*/
	
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radioid,	
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
		dbus_message_iter_get_basic(&iter,&num);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&auto_channel);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&diversity);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&txantenna);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		for (i = 0; i < num; i++) {
			DBusMessageIter iter_struct;
			
			BSS[i] = (WID_BSS *)malloc(sizeof(WID_BSS));
			BSS[i]->BSSID = (unsigned char *)malloc(6);
			memset(BSS[i]->BSSID,0,6);
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(BSS[i]->BSSIndex));

			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(BSS[i]->WlanID));
		
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(BSS[i]->BSSID[0]));

			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(BSS[i]->BSSID[1]));

			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(BSS[i]->BSSID[2]));

			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(BSS[i]->BSSID[3]));

			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(BSS[i]->BSSID[4]));

			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(BSS[i]->BSSID[5]));

			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(BSS[i]->vlanid));

			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(BSS[i]->wlan_vlanid));

			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(BSS[i]->State));

			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(BSS[i]->BSS_IF_POLICY));
			
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(BSS[i]->BSS_TUNNEL_POLICY));				

			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(BSS[i]->ath_l2_isolation));

			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(BSS[i]->cwmmode));

			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(BSS[i]->traffic_limit_able));

			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(BSS[i]->traffic_limit));

			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(BSS[i]->average_rate));

			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(BSS[i]->send_traffic_limit));

			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(BSS[i]->send_average_rate));

			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(BSS[i]->ip_mac_binding));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(nas_id));
			memset(BSS[i]->nas_id,0,NAS_IDENTIFIER_NAME);
			memcpy(BSS[i]->nas_id,nas_id,NAS_IDENTIFIER_NAME);
			dbus_message_iter_next(&iter_array);
		}
	}
		
		dbus_message_unref(reply);
#endif
		RADIOINFO = dcli_radio_show_api_group_one(
			index,
			radioid,/*"show radio bss list"*/
			&localid,
			&ret,
			0,
			//RADIOINFO,
			dcli_dbus_connection,
			WID_DBUS_RADIO_METHOD_SHOW_BSS_LIST
			);	
		vty_out(vty,"radio bss list summary:\n");
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"wtp id:	%d\n",radioid/4);
		vty_out(vty,"global radio id:	%d\n",radioid);
		vty_out(vty,"local radio id:	%d\n",radioid%4);
		vty_out(vty,"%d bss exist\n",RADIOINFO->bss_num_int);
		vty_out(vty,"auto channel switch:	%s\n",(RADIOINFO->RADIO[0]->auto_channel==1)?en:dis);
		vty_out(vty,"diversity switch:	%s\n",(RADIOINFO->RADIO[0]->diversity==1)?en:dis);
		vty_out(vty,"tx antenna:	%s\n",txantenna_policy[RADIOINFO->RADIO[0]->txantenna]);
		vty_out(vty,"==============================================================================\n");
		if(ret == -1){
			cli_syslog_info("<error> failed get reply.\n");
		}
		if(ret == 0){
			for (i = 0; i < RADIOINFO->bss_num_int; i++) 
			{	
				vty_out(vty,"bssindex:	%d\n",RADIOINFO->RADIO[0]->BSS[i]->BSSIndex);
				vty_out(vty,"BSSID:	%02X:%02X:%02X:%02X:%02X:%02X\n",RADIOINFO->RADIO[0]->BSS[i]->BSSID[0],
																RADIOINFO->RADIO[0]->BSS[i]->BSSID[1],
																RADIOINFO->RADIO[0]->BSS[i]->BSSID[2],
																RADIOINFO->RADIO[0]->BSS[i]->BSSID[3],
																RADIOINFO->RADIO[0]->BSS[i]->BSSID[4],
																RADIOINFO->RADIO[0]->BSS[i]->BSSID[5]);
				vty_out(vty,"wlan id:	%d\n",RADIOINFO->RADIO[0]->BSS[i]->WlanID);
				vty_out(vty,"state:	%s\n",(RADIOINFO->RADIO[0]->BSS[i]->State==1)?en:dis);
				vty_out(vty,"bss vlanid:	%d\n",RADIOINFO->RADIO[0]->BSS[i]->vlanid);
				vty_out(vty,"wlan vlanid:	%d\n",RADIOINFO->RADIO[0]->BSS[i]->wlan_vlanid);
				vty_out(vty,"bss hotspotid:  %d\n",RADIOINFO->RADIO[0]->BSS[i]->hotspot_id);
				vty_out(vty,"interface policy:	%s\n",if_policy[(RADIOINFO->RADIO[0]->BSS[i]->BSS_IF_POLICY)]);
				vty_out(vty,"forward policy:	%s\n",forward_policy[(RADIOINFO->RADIO[0]->BSS[i]->BSS_IF_POLICY)]);
				unsigned char bsspolicy = 0;
				bsspolicy = RADIOINFO->RADIO[0]->BSS[i]->BSS_TUNNEL_POLICY;
				
				vty_out(vty,"tunnel policy:	%s\n",(bsspolicy ==8 )?ipip:((bsspolicy ==2 )?capwap3:capwap11));
						
				vty_out(vty,"L2 isolation:	%s\n",(RADIOINFO->RADIO[0]->BSS[i]->ath_l2_isolation==1)?en:dis);
				vty_out(vty,"cwmmode:	%d\n",RADIOINFO->RADIO[0]->BSS[i]->cwmmode);
				vty_out(vty,"traffic limit:	%s\n",(RADIOINFO->RADIO[0]->BSS[i]->traffic_limit_able==1)?en:dis);
				vty_out(vty,"uplink traffic limit:	%d kbps\n",RADIOINFO->RADIO[0]->BSS[i]->traffic_limit);
				vty_out(vty,"uplink average traffic limit:	%d kbps\n",RADIOINFO->RADIO[0]->BSS[i]->average_rate);
				vty_out(vty,"downlink traffic limit:	%d kbps\n",RADIOINFO->RADIO[0]->BSS[i]->send_traffic_limit);
				vty_out(vty,"downlink average traffic limit:	%d kbps\n",RADIOINFO->RADIO[0]->BSS[i]->send_average_rate);
				vty_out(vty,"ip mac binding :	%s\n",(RADIOINFO->RADIO[0]->BSS[i]->ip_mac_binding==1)?en:dis);
				vty_out(vty,"interface nas_id:	%s\n",RADIOINFO->RADIO[0]->BSS[i]->nas_id);
				vty_out(vty,"---------------------------------------------------\n");
			/*	if(RADIOINFO->RADIO[0]->BSS[i])
				{
					free(RADIOINFO->RADIO[0]->BSS[i]->BSSID);
					RADIOINFO->RADIO[0]->BSS[i]->BSSID = NULL;
					free(RADIOINFO->RADIO[0]->BSS[i]);
					RADIOINFO->RADIO[0]->BSS[i] = NULL;	
				}*/
			}
			dcli_radio_free_fun(WID_DBUS_RADIO_METHOD_SHOW_BSS_LIST,RADIOINFO);
		}
		vty_out(vty,"==============================================================================\n");
		return CMD_SUCCESS;
	

}
#if _GROUP_POLICY
DEFUN(set_radio_l2_isolation_func,
	  set_radio_l2_isolation_cmd,
	  "set wlan ID l2 isolation (enable|disable)",
	  CONFIG_STR
	  "radio wlan config\n"
	  "wlan ID \n"
	  "radio l2 isolation config\n"
	  "radio l2 isolation config\n"
	  "radio l2 isolation enable|disable\n"
	 )
{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0; 
	unsigned char wlanid = 0;
    unsigned int policy = 0;

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 	

	ret = parse_char_ID((char*)argv[0], &wlanid);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
			vty_out(vty,"<error> unknown id format\n");
			}
			return CMD_SUCCESS;
	}	
	if(wlanid >= WLAN_NUM || wlanid == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
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
	
	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    } 
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = set_radio_l2_isolation_cmd_set_wlan_ID_l2_isolation(localid,index,dcli_dbus_connection,type,id,wlanid,policy,
	&count,&ret);

	if(type==0)
		{
			if(ret == 0)
				{
					vty_out(vty," set radio wlan %s l2 isolation %s successfully\n",argv[0],argv[1]);
				}
			else if (ret == WLAN_ID_NOT_EXIST)
				{
					vty_out(vty,"<error>  wlan %d not exist\n",wlanid);
				}
			else if (ret == WTP_IS_NOT_BINDING_WLAN_ID)
				{
					vty_out(vty,"<error>  wtp not binding wlan %d\n",wlanid);
				}
			else if (ret == WTP_NOT_IN_RUN_STATE)
					vty_out(vty,"<error> wtp id does not run\n");
			else if (ret == WTP_OVER_MAX_BSS_NUM)
				{
					vty_out(vty,"<error>  binding wlan error\n");
				}
			else
				{
					vty_out(vty,"<error>  %d\n",ret);
				}
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set radio wlan %s l2 isolation %s successfully\n",id,argv[0],argv[1]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}
	return CMD_SUCCESS;			
}

#else
DEFUN(set_radio_l2_isolation_func,
	  set_radio_l2_isolation_cmd,
	  "set wlan ID l2 isolation (enable|disable)",
	  CONFIG_STR
	  "radio wlan config\n"
	  "wlan ID \n"
	  "radio l2 isolation config\n"
	  "radio l2 isolation config\n"
	  "radio l2 isolation enable|disable\n"
	 )
{
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int radioID = 0;
	unsigned char wlanid = 0;
	int ret = WID_DBUS_SUCCESS;
	//radioID = (unsigned int)vty->index;
    int policy = 0;

	//wlanid = (unsigned char)atoi(argv[0]);
	ret = parse_char_ID((char*)argv[0], &wlanid);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
			vty_out(vty,"<error> unknown id format\n");
			}
			return CMD_SUCCESS;
	}	
	if(wlanid >= WLAN_NUM || wlanid == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
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
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radioID = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		radioID = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radioID = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_RADIO_L2_ISOLATION_ABLE);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_SET_RADIO_L2_ISOLATION_ABLE);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radioID,
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

	if(ret == 0)
	{
		vty_out(vty," set radio wlan %s l2 isolation %s successfully\n",argv[0],argv[1]);
	}
	else if (ret == WLAN_ID_NOT_EXIST)
	{
		vty_out(vty,"<error>  wlan %d not exist\n",wlanid);
	}
	else if (ret == WTP_IS_NOT_BINDING_WLAN_ID)
	{
		vty_out(vty,"<error>  wtp not binding wlan %d\n",wlanid);
	}
	else if (ret == WTP_NOT_IN_RUN_STATE)
		vty_out(vty,"<error> wtp id does not run\n");
	else if (ret == WTP_OVER_MAX_BSS_NUM)
	{
		vty_out(vty,"<error>  binding wlan error\n");
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
#endif
#if _GROUP_POLICY
DEFUN(set_radio_11n_cwmmode_func,
	  set_radio_11n_cwmmode_cmd,
	  "set 11n wlan ID cwmmode MODE",
	  CONFIG_STR
	  "11n config\n"
	  "radio wlan config\n"
	  "wlan ID \n"
	  "radio wlan cwmmode config\n"
	  "radio wlan cwmmode config 0--20mode 1 -- 40 mode\n"
	 )
{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0; 
	unsigned char wlanid = 0;
    unsigned char policy = 0;

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 	

	ret = parse_char_ID((char*)argv[0], &wlanid);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
			vty_out(vty,"<error> unknown id format\n");
			}
			return CMD_SUCCESS;
	}	
	if(wlanid >= WLAN_NUM || wlanid == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	
	if (!strcmp(argv[1],"1"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[1],"0"))
	{
		policy = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	

	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    } 
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = set_radio_11n_cwmmode_cmd_set_11n_wlan_ID_cwmmode(localid,index,dcli_dbus_connection,type,id,wlanid,policy,
	&count,&ret);

	if(type==0)
		{
			if(ret == 0)
				{
					vty_out(vty," set 11n radio wlan %s cwmmode %s successfully\n",argv[0],argv[1]);
				}
			else if (ret == WLAN_ID_NOT_EXIST)
				{
					vty_out(vty,"<error>  wlan %d not exist\n",wlanid);
				}
			else if (ret == WTP_IS_NOT_BINDING_WLAN_ID)
				{
					vty_out(vty,"<error>  wtp not binding wlan %d\n",wlanid);
				}
			else if (ret == WTP_NOT_IN_RUN_STATE)
					vty_out(vty,"<error> wtp id does not run\n");
			else if (ret == WTP_OVER_MAX_BSS_NUM)
				{
					vty_out(vty,"<error>  binding wlan error\n");
				}
			else
				{
					vty_out(vty,"<error>  %d\n",ret);
				}
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set 11n radio wlan %s cwmmode %s successfully\n",id,argv[0],argv[1]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}
		
	return CMD_SUCCESS;			
}

#else
DEFUN(set_radio_11n_cwmmode_func,
	  set_radio_11n_cwmmode_cmd,
	  "set 11n wlan ID cwmmode MODE",
	  CONFIG_STR
	  "11n config\n"
	  "radio wlan config\n"
	  "wlan ID \n"
	  "radio wlan cwmmode config\n"
	  "radio wlan cwmmode config 0--20mode 1 -- 40 mode\n"
	 )
{
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int radioID = 0;
	unsigned char wlanid = 0;
	int ret = WID_DBUS_SUCCESS;
	//radioID = (unsigned int)vty->index;
    unsigned char policy = 0;

	//wlanid = (unsigned char)atoi(argv[0]);
	ret = parse_char_ID((char*)argv[0], &wlanid);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
			vty_out(vty,"<error> unknown id format\n");
			}
			return CMD_SUCCESS;
	}	
	if(wlanid >= WLAN_NUM || wlanid == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	
	if (!strcmp(argv[1],"1"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[1],"0"))
	{
		policy = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radioID = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		radioID = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radioID = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_11N_SET_RADIO_CWMMODE);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_11N_SET_RADIO_CWMMODE);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radioID,
							 DBUS_TYPE_BYTE,&wlanid,
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
		vty_out(vty," set 11n radio wlan %s cwmmode %s successfully\n",argv[0],argv[1]);
	}
	else if (ret == WLAN_ID_NOT_EXIST)
	{
		vty_out(vty,"<error>  wlan %d not exist\n",wlanid);
	}
	else if (ret == WTP_IS_NOT_BINDING_WLAN_ID)
	{
		vty_out(vty,"<error>  wtp not binding wlan %d\n",wlanid);
	}
	else if (ret == WTP_NOT_IN_RUN_STATE)
		vty_out(vty,"<error> wtp id does not run\n");
	else if (ret == WTP_OVER_MAX_BSS_NUM)
	{
		vty_out(vty,"<error>  binding wlan error\n");
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
#endif
#if _GROUP_POLICY
DEFUN(radio_wlan_wds_bssid_cmd_func,
		radio_wlan_wds_bssid_cmd,
		"wlan WLANID (add|delete) wds_bssid MAC",
		"wlan configure\n"
		"wlan id\n" 
		"add/delete list\n" 
		"wds_bssid\n"
		"like: 00:19:E0:81:48:B5\n"
		)
{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0; 
	unsigned char mac[6];

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 	

	unsigned char wlan_id = 0;
	unsigned char list_type=0;   /*1--add list*/
								/* 2--delete list*/
	
	wlan_id = atoi(argv[0]);
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	str2lower(&argv[1]);
	
	if (!strcmp(argv[1],"add")||(tolower(argv[1][0]) == 'a')){
		list_type=1;		
	}else if (!strcmp(argv[1],"delete")||(tolower(argv[1][0]) == 'd')){
		list_type=2;		
	}else{
		vty_out(vty,"<error> input patameter should only be 'add/delelte' or 'a/d' \n");
		return CMD_SUCCESS;
	}
	
	ret = wid_parse_mac_addr((char *)argv[2],&mac);
	if (CMD_FAILURE == ret) {
		vty_out(vty,"<error> Unknown mac addr format.\n");
		return CMD_FAILURE;
	}
	
	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    } 
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = radio_wlan_wds_bssid_cmd_wlan_wds_bssid_MAC(localid,index,dcli_dbus_connection,type,id,mac,wlan_id,list_type,
	&count,&ret);

	if(type==0)
		{
			if(ret==WLAN_ID_NOT_EXIST)
				vty_out(vty,"<error> wlan isn't existed\n");
			else if(ret==RADIO_NO_BINDING_WLAN)
				vty_out(vty,"<error> radio doesn't bind wlan %s\n",argv[0]);
			else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
				vty_out(vty,"<error> radio doesn't bind wlan\n");
			else if(ret == WID_DBUS_SUCCESS)
				vty_out(vty,"WDS op successfully!\n");
			else if(ret == WDS_MODE_BE_USED)
				vty_out(vty,"another WDS mode be used, disable first\n");
			else
				vty_out(vty,"WDS op failed\n");
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d WDS op successfully!\n",id);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}
	return CMD_SUCCESS; 
}

#else
DEFUN(radio_wlan_wds_bssid_cmd_func,
		radio_wlan_wds_bssid_cmd,
		"wlan WLANID (add|delete) (wds_bssid|mesh_bssid) MAC",
		"wlan configure\n"
		"wlan id\n" 
		"add/delete list\n" 
		"wds_bssid\n"
		"like: 00:19:E0:81:48:B5\n"
		)
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char mac[6];
	unsigned int ret;
	unsigned int RadioID;
	unsigned char wlan_id = 0;
	unsigned char list_type=0;   /*1--add list*/
								/* 2--delete list*/
	
	//RadioID = (int)vty->index;
	wlan_id = atoi(argv[0]);
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	str2lower(&argv[1]);
	
	if (!strcmp(argv[1],"add")||(tolower(argv[1][0]) == 'a')){
		if(!strcmp(argv[2],"wds_bssid")){
				list_type=1;	
		}
		else if(!strcmp(argv[2],"mesh_bssid")){
			list_type = 5;
		}
		else{
			vty_out(vty,"<erro> input patameter should only be 'wds_bssid' or 'mesh_bssid'\n");
			return CMD_SUCCESS;
		}
	}else if (!strcmp(argv[1],"delete")||(tolower(argv[1][0]) == 'd')){
		if(!strcmp(argv[2],"wds_bssid")){
				list_type=2;	
		}
		else if(!strcmp(argv[2],"mesh_bssid")){
			list_type = 6;
		}
		else{
			vty_out(vty,"<erro> input patameter should only be 'wds_bssid' or 'mesh_bssid'\n");
			return CMD_SUCCESS;
		}	
	}else{
		vty_out(vty,"<error> input patameter should only be 'add/delelte' or 'a/d' \n");
		return CMD_SUCCESS;
	}
	
	ret = wid_parse_mac_addr((char *)argv[3],&mac);
	if (CMD_FAILURE == ret) {
		vty_out(vty,"<error> Unknown mac addr format.\n");
		return CMD_FAILURE;
	}
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		RadioID = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		RadioID = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		RadioID = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_WDS_WLAN_SET);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_WDS_WLAN_SET);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&RadioID,
							DBUS_TYPE_BYTE,&wlan_id,
							DBUS_TYPE_BYTE,&list_type,
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
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret==WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan isn't existed\n");
	else if(ret==RADIO_NO_BINDING_WLAN)
		vty_out(vty,"<error> radio doesn't bind wlan %s\n",argv[0]);
	else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
		vty_out(vty,"<error> radio doesn't bind wlan\n");
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else if(ret == WID_DBUS_SUCCESS)
	{
		if((list_type&0x04) == 0){
			vty_out(vty,"WDS op successfully!\n");
		}
		else{
			vty_out(vty,"Mesh op successfully!\n");
		}
	}
	else if(ret == WDS_MODE_BE_USED){
		if((list_type&0x04) == 0){
			vty_out(vty,"another WDS mode be used, disable first\n");
		}
		else{
			vty_out(vty,"another Mesh mode be used, disable first\n");
		}
	}	
	else{
		if((list_type&0x04) == 0){
			vty_out(vty,"WDS op failed\n");
		}
		else{
			vty_out(vty,"Mesh op faild\n");}
	}
	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}
#endif

DEFUN(show_wlan_wds_bssid_list_cmd_func,
	  show_wlan_wds_bssid_list_cmd,
	  "show wlan WLANID (wds_bssid_list|mesh_bssid_list)",
	  CONFIG_STR
	  "Wireless LAN WDS BSSID information\n"
	  "Wlan id that you want to config\n"
	 )
{	int ret,i;
	unsigned char wlan_id;
	unsigned int radioID;
	unsigned int num;
	unsigned char *mac;
	unsigned int wds_mesh = 0;
/*	DBusMessage *query, *reply;
	DBusError err;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	*/
	DCLI_RADIO_API_GROUP_ONE *RADIOINFO = NULL;
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
	if(!strcmp(argv[1],"wds_bssid_list")){
		wds_mesh = 0;
	}
	else if(!strcmp(argv[1],"mesh_bssid_list")){
		wds_mesh = 1;
	}
	else
	{
		vty_out(vty,"you should only 'wds_bssid_list' or 'mesh_bssid_list'\n");
	}
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radioID = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		radioID = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radioID = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#if 0	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SHOW_WDS_BSSID_INFO);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_SHOW_WDS_BSSID_INFO);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&wlan_id,
							 DBUS_TYPE_UINT32,&radioID,
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
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);	
		vty_out(vty,"WDS BSSID LIST INFO:\n");
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"RADIOID %d-%d WLANID %d BSSID_NUM %d\n",radioID/L_RADIO_NUM,radioID%L_RADIO_NUM, wlan_id, num);
		mac = (char*)malloc(MAC_LEN);		
		vty_out(vty,"==============================================================================\n");
		for (i = 0; i < num; i++) {
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
			
			vty_out(vty,"BSSID: %02X:%02X:%02X:%02X:%02X:%02X\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
		}
		free(mac);
		mac = NULL;
		vty_out(vty,"==============================================================================\n");
	}else if (ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan id does not exist\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
#endif
	RADIOINFO = dcli_radio_show_api_group_one(
		index,
		radioID,/*"show wlan WLANID wds_bssid_list"*/
		&localid,
		&ret,
		&wlan_id,
		//RADIOINFO,
		dcli_dbus_connection,
		WID_DBUS_RADIO_METHOD_SHOW_WDS_BSSID_INFO
		);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	if(ret == 0){
		if(0==wds_mesh){
			vty_out(vty,"WDS BSSID LIST INFO:\n");
		}
		else if(1 == wds_mesh){
			vty_out(vty,"Mesh BSSID LIST INFO:\n");
		}
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"RADIOID %d-%d WLANID %d BSSID_NUM %d\n",radioID/L_RADIO_NUM,radioID%L_RADIO_NUM, wlan_id, ((ret !=0)?0:RADIOINFO->bss_num_int));
		vty_out(vty,"==============================================================================\n");
		if((RADIOINFO)&&(RADIOINFO->BSS[0])&&(RADIOINFO->BSS[0]->wds_bss_list)){
			struct wds_bssid *head = NULL;
			head = RADIOINFO->BSS[0]->wds_bss_list;
			while(head){
				vty_out(vty,"BSSID: %02X:%02X:%02X:%02X:%02X:%02X\n",head->BSSID[0],head->BSSID[1],head->BSSID[2],head->BSSID[3],head->BSSID[4],head->BSSID[5]);
				head = head->next;
			}
		}
		//free(mac);
		//mac = NULL;
		vty_out(vty,"==============================================================================\n");
		//CW_FREE_OBJECT(RADIOINFO->BSS[0]->wds_bss_list);
		//CW_FREE_OBJECT(RADIOINFO->BSS[0]);		
		//CW_FREE_OBJECT(RADIOINFO->BSS);
		dcli_radio_free_fun(WID_DBUS_RADIO_METHOD_SHOW_WDS_BSSID_INFO,RADIOINFO);
	}else if (ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan id does not exist\n");
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	return CMD_SUCCESS;
}
#if _GROUP_POLICY
DEFUN(set_ap_radio_auto_channel_func,
	  set_ap_radio_auto_channel_cmd,
	  "set radio auto channel (enable|disable)",
	  CONFIG_STR
	  "ap radio auto channel\n"
	  "ap radio auto channel\n"
	  "ap radio auto channel\n"
	  "ap radio auto channel enable|disable\n"
	 )
{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned int policy =0;  

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 	

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
	
	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    } 
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = set_ap_radio_auto_channel_cmd_set_radio_auto_channel(localid,index,dcli_dbus_connection,type,id,policy,
	&count,&ret);

	if(type==0)
		{
			if(ret == 0)
				{
					vty_out(vty,"set ap radio auto channel %s successfully\n",argv[0]);
				}
			else if(ret == WTP_ID_NOT_EXIST)
				{	
					vty_out(vty,"<error> wtp id does not exist\n");
				}
			else if(ret == RADIO_ID_NOT_EXIST)
				{	
					vty_out(vty,"<error> radio id does not exist\n");
				}
			else
				{
					vty_out(vty,"<error>  %d\n",ret);
				}
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set ap radio auto channel %s successfully\n",id,argv[0]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}

	return CMD_SUCCESS;		
}

#else
DEFUN(set_ap_radio_auto_channel_func,
	  set_ap_radio_auto_channel_cmd,
	  "set radio auto channel (enable|disable)",
	  CONFIG_STR
	  "ap radio auto channel\n"
	  "ap radio auto channel\n"
	  "ap radio auto channel\n"
	  "ap radio auto channel enable|disable\n"
	 )
{
	int ret = 0;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	unsigned int radio_id = 0;
	unsigned int policy = 0;
	//radio_id = (unsigned int)vty->index;

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
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_WTP_RADIO_AUTO_CHANNEL);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_SET_WTP_RADIO_AUTO_CHANNEL);*/
	
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
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
		vty_out(vty,"set ap radio auto channel %s successfully\n",argv[0]);
	}
	else if(ret == WTP_ID_NOT_EXIST)
	{	
		vty_out(vty,"<error> wtp id does not exist\n");
	}
	else if(ret == RADIO_ID_NOT_EXIST)
	{	
		vty_out(vty,"<error> radio id does not exist\n");
	}
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	return CMD_SUCCESS;		
}
#endif
#if _GROUP_POLICY
DEFUN(set_ap_radio_auto_channel_cont_func,
	  set_ap_radio_auto_channel_cont_cmd,
	  "set radio auto channel_cont (enable|disable)",
	  CONFIG_STR
	  "ap radio auto channel_cont\n"
	  "ap radio auto channel_cont\n"
	  "ap radio auto channel_cont\n"
	  "ap radio auto channel_cont enable|disable\n"
	 )
{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned int policy =0;  

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 	

	if (!strcmp(argv[0],"enable"))
	{
		policy = 0;	
	}
	else if (!strcmp(argv[0],"disable"))
	{
		policy = 1;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}

	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
    } 
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = set_ap_radio_auto_channel_cont_cmd_set_radio_auto(localid,index,dcli_dbus_connection,type,id,policy,&count,&ret);

	if(type==0)
		{
			if(ret == 0)
				{
					vty_out(vty,"set ap radio auto channel %s successfully\n",argv[0]);
				}
			else if(ret == WTP_ID_NOT_EXIST)
				{	
					vty_out(vty,"<error> wtp id does not exist\n");
				}
			else if(ret == RADIO_ID_NOT_EXIST)
				{	
					vty_out(vty,"<error> radio id does not exist\n");
				}
			else
				{
					vty_out(vty,"<error>  %d\n",ret);
				}
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set ap radio auto channel %s successfully\n",id,argv[0]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}

	return CMD_SUCCESS;		
}

#else
DEFUN(set_ap_radio_auto_channel_cont_func,
	  set_ap_radio_auto_channel_cont_cmd,
	  "set radio auto channel_cont (enable|disable)",
	  CONFIG_STR
	  "ap radio auto channel_cont\n"
	  "ap radio auto channel_cont\n"
	  "ap radio auto channel_cont\n"
	  "ap radio auto channel_cont enable|disable\n"
	 )
{
	int ret = 0;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	unsigned int radio_id = 0;
	unsigned int policy = 0;
	radio_id = (unsigned int)vty->index;

	if (!strcmp(argv[0],"enable"))
	{
		policy = 0;	
	}
	else if (!strcmp(argv[0],"disable"))
	{
		policy = 1;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}

	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_WTP_RADIO_AUTO_CHANNEL_CONT);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_SET_WTP_RADIO_AUTO_CHANNEL_CONT);
	*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
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
		vty_out(vty,"set ap radio auto channel %s successfully\n",argv[0]);
	}
	else if(ret == WTP_ID_NOT_EXIST)
	{	
		vty_out(vty,"<error> wtp id does not exist\n");
	}
	else if(ret == RADIO_ID_NOT_EXIST)
	{	
		vty_out(vty,"<error> radio id does not exist\n");
	}
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	return CMD_SUCCESS;		
}
#endif
#if _GROUP_POLICY
DEFUN(set_ap_radio_diversity_func,
	  set_ap_radio_diversity_cmd,
	  "set radio diversity (enable|disable)",
	  CONFIG_STR
	  "ap radio diversity\n"
	  "ap radio diversity\n"
	  "ap radio diversity\n"
	  "ap radio diversity enable|disable\n"
	 )
{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned int policy =0;  

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 	


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
	

	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
    } 
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = set_ap_radio_diversity_cmd_set_radio_diversity(localid,index,dcli_dbus_connection,type,id,policy,&count,&ret);

	if(type==0)
		{
			if(ret == 0)
				{
					vty_out(vty,"set ap radio diversity %s successfully\n",argv[0]);
					if(policy == 1)
						{
							vty_out(vty,"to enable this function, you should restart wtp\n",argv[0]);
						}
				}
			else if(ret == WTP_ID_NOT_EXIST)
				{	
					vty_out(vty,"<error> wtp id does not exist\n");
				}
			else if(ret == RADIO_ID_NOT_EXIST)
				{	
					vty_out(vty,"<error> radio id does not exist\n");
				}
			else if(ret == MODEL_NO_EXIST)
				{	
					vty_out(vty,"<error> radio model not petmit to set diversity\n");
				}
			else
				{
					vty_out(vty,"<error>  %d\n",ret);
				}	
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set ap radio diversity %s successfully\n",id,argv[0]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}
	return CMD_SUCCESS;		
}

#else
DEFUN(set_ap_radio_diversity_func,
	  set_ap_radio_diversity_cmd,
	  "set radio diversity (enable|disable)",
	  CONFIG_STR
	  "ap radio diversity\n"
	  "ap radio diversity\n"
	  "ap radio diversity\n"
	  "ap radio diversity enable|disable\n"
	 )
{
	int ret = 0;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	unsigned int radio_id = 0;
	unsigned int policy = 0;
	//radio_id = (unsigned int)vty->index;

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
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_WTP_RADIO_DIVERSITY);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_SET_WTP_RADIO_DIVERSITY);*/
	
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
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
		vty_out(vty,"set ap radio diversity %s successfully\n",argv[0]);
		if(policy == 1)
		{
			vty_out(vty,"to enable this function, you should restart wtp\n",argv[0]);
		}
	}
	else if(ret == WTP_ID_NOT_EXIST)
	{	
		vty_out(vty,"<error> wtp id does not exist\n");
	}
	else if(ret == RADIO_ID_NOT_EXIST)
	{	
		vty_out(vty,"<error> radio id does not exist\n");
	}
	else if(ret == MODEL_NO_EXIST)
	{	
		vty_out(vty,"<error> radio model not petmit to set diversity\n");
	}
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	return CMD_SUCCESS;		
}
#endif
#if _GROUP_POLICY
DEFUN(set_ap_radio_txantenna_func,
	  set_ap_radio_txantenna_cmd,
	  "set radio txantenna (auto|main|vice)",
	  CONFIG_STR
	  "ap radio txantenna\n"
	  "ap radio txantenna\n"
	  "ap radio txantenna\n"
	  "ap radio txantenna auto|main|vice\n"
	 )
{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned int policy =0;  

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 	

	if (!strcmp(argv[0],"auto"))
	{
		policy = 0;	
	}
	else if (!strcmp(argv[0],"main"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[0],"vice"))
	{
		policy = 2;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'auto' 'main' or 'vice'\n");
		return CMD_SUCCESS;
	}
	
	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    } 
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = set_ap_radio_txantenna_cmd_set_radio_txantenna(localid,index,dcli_dbus_connection,type,id,policy,
	&count,&ret);
	
	if(type==0)
		{
			if(ret == 0)
				{
					vty_out(vty,"set ap radio txantenna %s successfully\n",argv[0]);
				}
			else if(ret == WTP_ID_NOT_EXIST)
				{	
					vty_out(vty,"<error> wtp id does not exist\n");
				}
			else if(ret == RADIO_ID_NOT_EXIST)
				{	
					vty_out(vty,"<error> radio id does not exist\n");
				}
			else
				{
					vty_out(vty,"<error>  %d\n",ret);
				}	
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set ap radio txantenna %s successfully\n",id,argv[0]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}

	return CMD_SUCCESS;		
}

#else
DEFUN(set_ap_radio_txantenna_func,
	  set_ap_radio_txantenna_cmd,
	  "set radio txantenna (auto|main|vice)",
	  CONFIG_STR
	  "ap radio txantenna\n"
	  "ap radio txantenna\n"
	  "ap radio txantenna\n"
	  "ap radio txantenna auto|main|vice\n"
	 )
{
	int ret = 0;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	unsigned int radio_id = 0;
	unsigned int policy = 0;
	//radio_id = (unsigned int)vty->index;

	if (!strcmp(argv[0],"auto"))
	{
		policy = 0;	
	}
	else if (!strcmp(argv[0],"main"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[0],"vice"))
	{
		policy = 2;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'auto' 'main' or 'vice'\n");
		return CMD_SUCCESS;
	}
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_WTP_RADIO_TXANTENNA);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_SET_WTP_RADIO_TXANTENNA);*/
	
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
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
		vty_out(vty,"set ap radio txantenna %s successfully\n",argv[0]);
	}
	else if(ret == WTP_ID_NOT_EXIST)
	{	
		vty_out(vty,"<error> wtp id does not exist\n");
	}
	else if(ret == RADIO_ID_NOT_EXIST)
	{	
		vty_out(vty,"<error> radio id does not exist\n");
	}
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	return CMD_SUCCESS;		
}
#endif
#if _GROUP_POLICY
/*traffic area*/
DEFUN(radio_bss_taffic_limit_func,
		radio_bss_taffic_limit_cmd,
		"wlan ID traffic limit (enable|disable)",
		"wlan configure\n"
		"wlan id\n" 
		"traffic limit\n" 
		"traffic limit\n"
		"traffic limit enable|disable \n"
		)
{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned char wlan_id = 0;
	unsigned char policy =0;  

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 		

	wlan_id = atoi(argv[0]);
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
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
	
	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    } 
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	RadioList_Head = radio_bss_taffic_limit_cmd_wlan_ID_traffic_limit(localid,index,dcli_dbus_connection,type,id,wlan_id,policy,
	&count,&ret);

	if(type==0)
		{
			if(ret==WLAN_ID_NOT_EXIST)
				vty_out(vty,"<error> wlan isn't existed\n");
			else if(ret == WTP_ID_NOT_EXIST)
				vty_out(vty,"<error> wtp isn't existed\n");
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio isn't existed\n");
			else if(ret==RADIO_NO_BINDING_WLAN)
				vty_out(vty,"<error> radio doesn't bind wlan %s\n",argv[0]);
			else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
				vty_out(vty,"<error> radio doesn't bind wlan\n");
			else if(ret == WID_DBUS_SUCCESS)
				vty_out(vty,"set wlan %s traffic limit %s successfully!\n",argv[0],argv[1]);
			else
				vty_out(vty,"<error> other error\n");	
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set wlan %s traffic limit %s successfully!\n",id,argv[0],argv[1]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}
	return CMD_SUCCESS;

}

#else
/*traffic area*/
DEFUN(radio_bss_taffic_limit_func,
		radio_bss_taffic_limit_cmd,
		"wlan ID traffic limit (enable|disable)",
		"wlan configure\n"
		"wlan id\n" 
		"traffic limit\n" 
		"traffic limit\n"
		"traffic limit enable|disable \n"
		)
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
	unsigned int RadioID;
	unsigned char wlan_id = 0;
	unsigned char policy =0;   
	
	//RadioID = (int)vty->index;
	wlan_id = atoi(argv[0]);
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
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
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		RadioID = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		RadioID = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		RadioID = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_ABLE);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_ABLE);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&RadioID,
							DBUS_TYPE_BYTE,&wlan_id,
							DBUS_TYPE_BYTE,&policy,
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
	
	if(ret==WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan isn't existed\n");
	else if(ret == WTP_ID_NOT_EXIST)
		vty_out(vty,"<error> wtp isn't existed\n");
	else if(ret == RADIO_ID_NOT_EXIST)
		vty_out(vty,"<error> radio isn't existed\n");
	else if(ret==RADIO_NO_BINDING_WLAN)
		vty_out(vty,"<error> radio doesn't bind wlan %s\n",argv[0]);
	else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
		vty_out(vty,"<error> radio doesn't bind wlan\n");
	else if(ret == WID_DBUS_SUCCESS)
		vty_out(vty,"set wlan %s traffic limit %s successfully!\n",argv[0],argv[1]);
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error> other error\n");
	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}
#endif
#if _GROUP_POLICY
DEFUN(radio_bss_taffic_limit_value_func,
		radio_bss_taffic_limit_value_cmd,
		"wlan ID traffic limit value VALUE",
		"wlan configure\n"
		"wlan id\n" 
		"traffic limit\n" 
		"traffic limit\n"
		"traffic limit value \n"
		"traffic limit value (kbps)\n"
		)
{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned char wlan_id = 0;
	unsigned int value = 0;  

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 	

	wlan_id = atoi(argv[0]);
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	ret = parse_int_ID((char *)argv[1],&value);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[1]);
		return CMD_SUCCESS;
	} 
	
	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    } 
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = radio_bss_taffic_limit_value_cmd_wlan_ID_traffic_limit_value(localid,index,dcli_dbus_connection,type,id,wlan_id,value,
	&count,&ret);

	if(type==0)
		{
			if(ret==WLAN_ID_NOT_EXIST)
				vty_out(vty,"<error> wlan isn't existed\n");
			else if(ret == WTP_ID_NOT_EXIST)
				vty_out(vty,"<error> wtp isn't existed\n");
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio isn't existed\n");
			else if(ret==RADIO_NO_BINDING_WLAN)
				vty_out(vty,"<error> radio doesn't bind wlan %s\n",argv[0]);
			else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
				vty_out(vty,"<error> radio doesn't bind wlan\n");
			else if(ret == WID_DBUS_SUCCESS)
				vty_out(vty,"set wlan %s traffic limit value %s successfully!\n",argv[0],argv[1]);
			else
				vty_out(vty,"<error> other error\n");	
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set wlan %s traffic limit value %s successfully!\n",id,argv[0],argv[1]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}

	return CMD_SUCCESS; 
}

#else
DEFUN(radio_bss_taffic_limit_value_func,
		radio_bss_taffic_limit_value_cmd,
		"wlan ID traffic limit value VALUE",
		"wlan configure\n"
		"wlan id\n" 
		"traffic limit\n" 
		"traffic limit\n"
		"traffic limit value \n"
		"traffic limit value (kbps)\n"
		)
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
	unsigned int RadioID;
	unsigned char wlan_id = 0;
	unsigned int value = 0;   
	
	//RadioID = (int)vty->index;
	wlan_id = atoi(argv[0]);
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	ret = parse_int_ID((char *)argv[1],&value);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[1]);
		return CMD_SUCCESS;
	}
	if((value > 884736)||(value < 1))      /*fengwenchao add 20110228  108*1024*8 = 884736*/
	{
		vty_out(vty,"<error> input value should be 1 to 884736\n");
		return CMD_SUCCESS;
	}
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		RadioID = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index;
		localid = vty->local;
        slot_id = vty->slotindex;
		RadioID = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		RadioID = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_VALUE);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_VALUE);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&RadioID,
							DBUS_TYPE_BYTE,&wlan_id,
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
	
	if(ret==WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan isn't existed\n");
	else if(ret == WTP_ID_NOT_EXIST)
		vty_out(vty,"<error> wtp isn't existed\n");
	else if(ret == RADIO_ID_NOT_EXIST)
		vty_out(vty,"<error> radio isn't existed\n");
	else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)  //fengwenchao modify 20120428 for autelan-2905
		vty_out(vty,"<error> radio doesn't bind wlan %s\n",argv[0]);
	else if(ret == WID_DBUS_SUCCESS)
		vty_out(vty,"set wlan %s traffic limit value %s successfully!\n",argv[0],argv[1]);
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error> other error\n");
	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}
#endif
#if _GROUP_POLICY
DEFUN(radio_bss_taffic_limit_average_value_func,
		radio_bss_taffic_limit_average_value_cmd,
		"wlan ID traffic limit station average value VALUE",
		"wlan configure\n"
		"wlan id\n" 
		"traffic limit\n" 
		"traffic limit\n"
		"traffic limit station value \n"
		"traffic limit station average value\n"
		"traffic limit station average value \n"
		"traffic limit station average value (kbps)\n"
		)
{ 
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned char wlan_id = 0;
	unsigned int value = 0;  

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 	

	wlan_id = atoi(argv[0]);
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	ret = parse_int_ID((char *)argv[1],&value);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[1]);
		return CMD_SUCCESS;
	} 
	if((value > 884736)||(value < 1))      /*fengwenchao add 20110228  108*1024*8 = 884736*/
	{
		vty_out(vty,"<error> input value should be 1 to 884736\n");
		return CMD_SUCCESS;
	}	
	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    } 
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = radio_bss_taffic_limit_average_value_cmd_wlan_ID_traffic_limit_station_average_value(localid,index,dcli_dbus_connection,type,id,
	wlan_id,value,&count,&ret);

	if(type==0)
		{
			if(ret==WLAN_ID_NOT_EXIST)
				vty_out(vty,"<error> wlan isn't existed\n");
			else if(ret == WTP_ID_NOT_EXIST)
				vty_out(vty,"<error> wtp isn't existed\n");
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio isn't existed\n");
			else if(ret==RADIO_NO_BINDING_WLAN)
				vty_out(vty,"<error> radio doesn't bind wlan %s\n",argv[0]);
			else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
				vty_out(vty,"<error> radio doesn't bind wlan\n");
			else if(ret == WID_DBUS_SUCCESS)
				vty_out(vty,"set wlan %s traffic limit station average value %s successfully!\n",argv[0],argv[1]);
			else if(ret == IF_POLICY_CONFLICT)
				vty_out(vty,"<error> station traffic limit value is more than bss traffic limit value\n");
			else
				vty_out(vty,"<error> other error\n");	
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set wlan %s traffic limit station average value %s successfully!\n",id,argv[0],argv[1]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}
	return CMD_SUCCESS; 
}

#else
DEFUN(radio_bss_taffic_limit_average_value_func,
		radio_bss_taffic_limit_average_value_cmd,
		"wlan ID traffic limit station average value VALUE",
		"wlan configure\n"
		"wlan id\n" 
		"traffic limit\n" 
		"traffic limit\n"
		"traffic limit station value \n"
		"traffic limit station average value\n"
		"traffic limit station average value \n"
		"traffic limit station average value (kbps)\n"
		)
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
	unsigned int RadioID;
	unsigned char wlan_id = 0;
	unsigned int value = 0;   
	
	//RadioID = (int)vty->index;
	wlan_id = atoi(argv[0]);
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	ret = parse_int_ID((char *)argv[1],&value);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[1]);
		return CMD_SUCCESS;
	} 
	if((value > 884736)||(value < 1))      /*fengwenchao add 20110228  108*1024*8 = 884736*/
	{
		vty_out(vty,"<error> input value should be 1 to 884736\n");
		return CMD_SUCCESS;
	}	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		RadioID = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		RadioID = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		RadioID = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_AVERAGE_VALUE);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_AVERAGE_VALUE);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&RadioID,
							DBUS_TYPE_BYTE,&wlan_id,
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
	
	if(ret==WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan isn't existed\n");
	else if(ret == WTP_ID_NOT_EXIST)
		vty_out(vty,"<error> wtp isn't existed\n");
	else if(ret == RADIO_ID_NOT_EXIST)
		vty_out(vty,"<error> radio isn't existed\n");
	else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)   //fengwenchao add 20120428 for autelan-2905
		vty_out(vty,"<error> radio doesn't bind wlan %s\n",argv[0]);
	else if(ret == WID_DBUS_SUCCESS)
		vty_out(vty,"set wlan %s traffic limit station average value %s successfully!\n",argv[0],argv[1]);
	else if(ret == IF_POLICY_CONFLICT)
		vty_out(vty,"<error> station traffic limit value is more than bss traffic limit value\n");
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error> other error\n");
	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}
#endif
/*fengwenchao add 20130416 for AXSSZFI-1374*/
DEFUN(radio_bss_taffic_limit_cancel_average_value_func,
		radio_bss_taffic_limit_cancel_average_value_cmd,
		"wlan ID traffic limit cancel station average value",
		"wlan configure\n"
		"wlan id\n" 
		"traffic limit\n" 
		"traffic limit\n"
		"traffic limit station value \n"
		"traffic limit station average value\n"
		"traffic limit station average value \n"
		"traffic limit station average value (kbps)\n"
		)
{

	DBusMessage *query = NULL, *reply = NULL;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret = 0;
	unsigned int RadioID;
	unsigned char wlan_id = 0;
	//unsigned int value = 0;   
	
	wlan_id = atoi(argv[0]);
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	#if 0

	ret = parse_int_ID((char *)argv[1],&value);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[1]);
		return CMD_SUCCESS;
	} 
	
	if((value > 884736)||(value < 1))      /*fengwenchao add 20110228  108*1024*8 = 884736*/
	{
		vty_out(vty,"<error> input value should be 1 to 884736\n");
		return CMD_SUCCESS;
	}	
	#endif
	int index = 0;
	int localid = 1;
    	int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		RadioID = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        		slot_id = vty->slotindex;
		RadioID = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
	         index = vty->index;
	         localid = vty->local;
	         slot_id = vty->slotindex;
		RadioID = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_CANCEL_AVERAGE_VALUE);
	

	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&RadioID,
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
	
	if(ret==WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan isn't existed\n");
	else if(ret == WTP_ID_NOT_EXIST)
		vty_out(vty,"<error> wtp isn't existed\n");
	else if(ret == RADIO_ID_NOT_EXIST)
		vty_out(vty,"<error> radio isn't existed\n");
	else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)   
		vty_out(vty,"<error> radio doesn't bind wlan %s\n",argv[0]);
	else if(ret == WID_DBUS_SUCCESS)
		vty_out(vty,"set wlan %s traffic limit cancel station average value successfully!\n",argv[0]);
	else if(ret == IF_POLICY_CONFLICT)
		vty_out(vty,"<error> station traffic limit value is more than bss traffic limit value\n");
	else
		vty_out(vty,"<error> other error\n");
	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}

DEFUN(radio_bss_taffic_limit_cancel_average_send_value_func,
		radio_bss_taffic_limit_cancel_average_send_value_cmd,
		"wlan ID traffic limit cancel station average send value",
		"wlan configure\n"
		"wlan id\n" 
		"traffic limit\n" 
		"traffic limit\n"
		"traffic limit station value \n"
		"traffic limit station average value\n"
		"traffic limit station average value \n"
		"traffic limit station average value (kbps)\n"
		)
{

	DBusMessage *query = NULL, *reply = NULL;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret = 0;
	unsigned int RadioID;
	unsigned char wlan_id = 0;
	
	wlan_id = atoi(argv[0]);
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	#if 0
	ret = parse_int_ID((char *)argv[1],&value);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[1]);
		return CMD_SUCCESS;
	} 
	if((value > 884736)||(value < 1))      /*fengwenchao add 20110228  108*1024*8 = 884736*/
	{
		vty_out(vty,"<error> input value should be 1 to 884736\n");
		return CMD_SUCCESS;
	}
	#endif
	int index = 0;
	int localid = 1;
    	int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		RadioID = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        		slot_id = vty->slotindex;
		RadioID = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
	         index = vty->index;
	         localid = vty->local;
	         slot_id = vty->slotindex;
		RadioID = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_CANCEL_AVERAGE_SEND_VALUE);
	

	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&RadioID,
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
	
	if(ret==WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan isn't existed\n");
	else if(ret == WTP_ID_NOT_EXIST)
		vty_out(vty,"<error> wtp isn't existed\n");
	else if(ret == RADIO_ID_NOT_EXIST)
		vty_out(vty,"<error> radio isn't existed\n");
	else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)   
		vty_out(vty,"<error> radio doesn't bind wlan %s\n",argv[0]);
	else if(ret == WID_DBUS_SUCCESS)
		vty_out(vty,"set wlan %s traffic limit cancel station average send value successfully!\n",argv[0]);
	else if(ret == IF_POLICY_CONFLICT)
		vty_out(vty,"<error> station traffic limit value is more than bss traffic limit value\n");
	else
		vty_out(vty,"<error> other error\n");
	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}

/*fengwenchao add end*/
#if 1
DEFUN(radio_bss_taffic_limit_sta_value_func,
		radio_bss_taffic_limit_sta_value_cmd,
		"wlan ID traffic limit station MAC value VALUE",
		"traffic limit station MAC xx:xx:xx:xx:xx:xx\n"
		"traffic limit station value (kbps)\n"
		)
{	
	 
	unsigned int	value = 0;
	unsigned int	mac[MAC_LEN]={0};
	unsigned char	mac1[MAC_LEN]={0};
	unsigned int	ret=0;
	unsigned int 	RadioID = 0;
	unsigned char 	wlan_id = 0;
	unsigned char	type = 1;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	
	wlan_id = atoi(argv[0]);
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	memset(mac,0,MAC_LEN);
	sscanf(argv[1],"%X:%X:%X:%X:%X:%X",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
	mac1[0] = (unsigned char)mac[0];
	mac1[1] = (unsigned char)mac[1];	
	mac1[2] = (unsigned char)mac[2];	
	mac1[3] = (unsigned char)mac[3];	
	mac1[4] = (unsigned char)mac[4];	
	mac1[5] = (unsigned char)mac[5];

	ret = parse_int_ID((char *)argv[2],&value);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[2]);
		return CMD_SUCCESS;
	} 
	if((value > 884736)||(value < 1))      /*fengwenchao add 20110228  108*1024*8 = 884736*/
	{
		vty_out(vty,"<error> input value should be 1 to 884736\n");
		return CMD_SUCCESS;
	}	
	if(vty->node == RADIO_NODE){
		index = 0;			
		RadioID = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		RadioID = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		RadioID = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	asd_set_sta_info(dcli_dbus_connection,index,mac1,wlan_id,RadioID,type,value,localid,&ret);

	if(ret == 0){
		wid_set_sta_info(dcli_dbus_connection,index,mac1,wlan_id,RadioID,type,value,localid,&ret);
		if(ret == 0){
			set_sta_static_info(dcli_dbus_connection,index,mac1,wlan_id,RadioID,type,value,localid,&ret);
			if(ret == 0){
				vty_out(vty,"set static info successfully.\n");
			}else if (ret == ASD_DBUS_ERROR)
				vty_out(vty,"<error> set info failed get reply.\n");
			else
				vty_out(vty,"<error> set info ret = %d.\n",ret);
		}else if (ret == ASD_DBUS_ERROR)
			vty_out(vty,"<error> wid set failed get reply.\n");
		else
			vty_out(vty,"<error> wid set error ret = %d\n",ret);
	}
	else if(ret==ASD_WLAN_NOT_EXIST)
		vty_out(vty,"<error> wlan doesn't work.\n");
	else if(ret == ASD_STA_NOT_EXIST)
		vty_out(vty,"<error> can't find sta under wlan %u.\n",wlan_id);
	else if(ret == ASD_DBUS_ERROR)
		vty_out(vty,"<error> value great than bss traffic limit.\n");
	else
		vty_out(vty,"<error> asd set ret = %d.\n",ret);

	return CMD_SUCCESS; 
}
#else
DEFUN(radio_bss_taffic_limit_sta_value_func,
		radio_bss_taffic_limit_sta_value_cmd,
		"wlan ID traffic limit station MAC value VALUE",
		"wlan configure\n"
		"wlan id\n" 
		"traffic limit\n" 
		"traffic limit\n"
		"traffic limit station value \n"
		"traffic limit station MAC xx:xx:xx:xx:xx:xx\n"
		"traffic limit station value \n"
		"traffic limit station value (kbps)\n"
		)
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
	unsigned int RadioID;
	unsigned char wlan_id = 0;
	unsigned int value = 0;   
	unsigned char mac1[MAC_LEN];
	unsigned int mac[MAC_LEN];
	
	//RadioID = (int)vty->index;
	wlan_id = atoi(argv[0]);
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	memset(mac,0,MAC_LEN);
	sscanf(argv[1],"%X:%X:%X:%X:%X:%X",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
	mac1[0] = (unsigned char)mac[0];
	mac1[1] = (unsigned char)mac[1];	
	mac1[2] = (unsigned char)mac[2];	
	mac1[3] = (unsigned char)mac[3];	
	mac1[4] = (unsigned char)mac[4];	
	mac1[5] = (unsigned char)mac[5];


	
	ret = parse_int_ID((char *)argv[2],&value);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[2]);
		return CMD_SUCCESS;
	} 
	
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		RadioID = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		RadioID = (int)vty->index_sub;
	}
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_STA_TRAFFIC_LIMIT);

/*	query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_STA_TRAFFIC_LIMIT);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&RadioID,
							DBUS_TYPE_BYTE,&wlan_id,
							DBUS_TYPE_UINT32,&value,
							DBUS_TYPE_BYTE,&mac1[0],
							DBUS_TYPE_BYTE,&mac1[1],
							DBUS_TYPE_BYTE,&mac1[2],
							DBUS_TYPE_BYTE,&mac1[3],
							DBUS_TYPE_BYTE,&mac1[4],
							DBUS_TYPE_BYTE,&mac1[5],
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
	
	if(ret==ASD_WLAN_NOT_EXIST)
		vty_out(vty,"<error> wlan doesn't work.\n");
	else if(ret == ASD_STA_NOT_EXIST)
		vty_out(vty,"<error> can't find sta under wlan %u\n",wlan_id);
	else  if(ret == ASD_DBUS_ERROR)
		vty_out(vty,"<error> value great than bss traffic limit .\n");
	
	dbus_message_unref(reply);


	if(ret == ASD_DBUS_SUCCESS)
	{
	
	/*	int index = 0;
		char BUSNAME[PATH_LEN];
		char OBJPATH[PATH_LEN];
		char INTERFACE[PATH_LEN];
		if(vty->node == RADIO_NODE){
			index = 0;			
			RadioID = (int)vty->index;
		}else if(vty->node == HANSI_RADIO_NODE){
			index = vty->index; 		
			RadioID = (int)vty->index_sub;
		}*/
		memset(BUSNAME,0,PATH_LEN);
		memset(OBJPATH,0,PATH_LEN);
		memset(INTERFACE,0,PATH_LEN);

		ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
		ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
		ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_STA_VALUE);

/*		query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_STA_VALUE);*/
		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_BYTE,&wlan_id,
								DBUS_TYPE_UINT32,&RadioID,
								DBUS_TYPE_UINT32,&value,
								DBUS_TYPE_BYTE,&mac1[0],
								DBUS_TYPE_BYTE,&mac1[1],
								DBUS_TYPE_BYTE,&mac1[2],
								DBUS_TYPE_BYTE,&mac1[3],
								DBUS_TYPE_BYTE,&mac1[4],
								DBUS_TYPE_BYTE,&mac1[5],
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

		if(ret==WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan isn't existed\n");
		else if(ret == WTP_ID_NOT_EXIST)
			vty_out(vty,"<error> wtp isn't existed\n");
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio isn't existed\n");
		else if(ret == IF_POLICY_CONFLICT)
			vty_out(vty,"<error> station traffic limit value is more than bss traffic limit value\n");
		else if(ret==RADIO_NO_BINDING_WLAN)
			vty_out(vty,"<error> radio doesn't bind wlan %s\n",argv[0]);
		else if(ret == WID_DBUS_SUCCESS)
			vty_out(vty,"set wlan %s traffic limit station %s value %s successfully!\n",argv[0],argv[1],argv[2]);
		else
			vty_out(vty,"<error> other error\n");
		dbus_message_unref(reply);
	}

	return CMD_SUCCESS; 
}
#endif

#if _GROUP_POLICY
DEFUN(radio_bss_taffic_limit_cancel_sta_value_func,
		radio_bss_taffic_limit_cancel_sta_value_cmd,
		"wlan ID traffic limit cancel station MAC value",
		"wlan configure\n"
		"wlan id\n" 
		"traffic limit\n" 
		"traffic limit\n"
		"traffic limit cancel station value \n"
		"traffic limit cancel station MAC xx:xx:xx:xx:xx:xx\n"
		"traffic limit cancel station value \n"
		)
{
	int i = 0;
	int ret = 0;
	int ret_asd = 0;
	int ret_wid = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	int failnum = 0;
	unsigned int id = 0;
	unsigned int type = 0;
  	unsigned char wlan_id = 0;
	unsigned char cancel_flag = 0;
	unsigned char mac1[MAC_LEN];
	unsigned int mac[MAC_LEN];

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 	

	wlan_id = atoi(argv[0]);
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	memset(mac,0,MAC_LEN);
	sscanf(argv[1],"%X:%X:%X:%X:%X:%X",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
	mac1[0] = (unsigned char)mac[0];
	mac1[1] = (unsigned char)mac[1];	
	mac1[2] = (unsigned char)mac[2];	
	mac1[3] = (unsigned char)mac[3];	
	mac1[4] = (unsigned char)mac[4];	
	mac1[5] = (unsigned char)mac[5];

	
	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    } 
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = radio_bss_taffic_limit_cancel_sta_value_cmd_wlan_configure(localid,index,dcli_dbus_connection,type,id,wlan_id,
	mac1,&count,&ret,&ret_asd,&ret_wid,&failnum);

	if(type==0)
		{
			if(ret == WID_DBUS_SUCCESS)
				{
					if(ret_asd == ASD_WLAN_NOT_EXIST)
						vty_out(vty,"<error> wlan doesn't work.\n");	
					else if(ret_asd == ASD_STA_NOT_EXIST)
						vty_out(vty,"<error> can't find sta under wlan %u\n",wlan_id);
					else if(ret_asd != ASD_DBUS_SUCCESS)
						vty_out(vty,"<error> other error %d(asd)\n",ret_asd);
					if(ret_asd == ASD_DBUS_SUCCESS)
						{
							if(ret_wid == WID_DBUS_SUCCESS)
								vty_out(vty,"set wlan %s traffic limit cancel station %s send value successfully!\n",argv[0],argv[1]);
							else if(ret_wid == WTP_ID_NOT_EXIST)
								vty_out(vty,"<error> wtp isn't existed\n");
							else if(ret_wid == RADIO_ID_NOT_EXIST)
								vty_out(vty,"<error> radio isn't existed\n");
							else if(ret_wid==RADIO_NO_BINDING_WLAN)
								vty_out(vty,"<error> radio doesn't bind wlan %s\n",argv[0]);
							else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
								vty_out(vty,"<error> radio doesn't bind wlan\n");
							else if(ret_wid==WLAN_ID_NOT_EXIST)
								vty_out(vty,"<error> wlan isn't existed\n");
							else
								vty_out(vty,"<error> other error %d(wid)\n",ret);
						}
				}
			else
				{
					vty_out(vty,"<error> radioid is not exist.\n");
				}
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set wlan %s traffic limit cancel station %s send value successfully!\n",id,argv[0],argv[1]);
					if((failnum != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<failnum; i++)
								{
									vty_out(vty,"%d ",RadioList_Head[i].RadioId);	
									vty_out(vty,"%d ",RadioList_Head[i].FailReason);		
								}
							vty_out(vty," failed.\n");
							if(RadioList_Head != NULL)
								{
									free(RadioList_Head);
									RadioList_Head = NULL;
								}
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");		
		}	
	return CMD_SUCCESS; 
}

#else
DEFUN(radio_bss_taffic_limit_cancel_sta_value_func,
		radio_bss_taffic_limit_cancel_sta_value_cmd,
		"wlan ID traffic limit cancel station MAC value",
		"wlan configure\n"
		"wlan id\n" 
		"traffic limit\n" 
		"traffic limit\n"
		"traffic limit cancel station value \n"
		"traffic limit cancel station MAC xx:xx:xx:xx:xx:xx\n"
		"traffic limit cancel station value \n"
		)
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
	unsigned int RadioID;
	unsigned char wlan_id = 0;
	unsigned char cancel_flag = 0;
	unsigned int value = 0;   
	unsigned char mac1[MAC_LEN];
	unsigned int mac[MAC_LEN];
	
	//RadioID = (int)vty->index;
	wlan_id = atoi(argv[0]);
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	memset(mac,0,MAC_LEN);
	sscanf(argv[1],"%X:%X:%X:%X:%X:%X",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
	mac1[0] = (unsigned char)mac[0];
	mac1[1] = (unsigned char)mac[1];	
	mac1[2] = (unsigned char)mac[2];	
	mac1[3] = (unsigned char)mac[3];	
	mac1[4] = (unsigned char)mac[4];	
	mac1[5] = (unsigned char)mac[5];

	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		RadioID = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		RadioID = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		RadioID = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_STA_TRAFFIC_LIMIT_CANCEL);

/*	query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_STA_TRAFFIC_LIMIT_CANCEL);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&RadioID,
							DBUS_TYPE_BYTE,&wlan_id,
							DBUS_TYPE_BYTE,&mac1[0],
							DBUS_TYPE_BYTE,&mac1[1],
							DBUS_TYPE_BYTE,&mac1[2],
							DBUS_TYPE_BYTE,&mac1[3],
							DBUS_TYPE_BYTE,&mac1[4],
							DBUS_TYPE_BYTE,&mac1[5],
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
	dbus_message_iter_get_basic(&iter,&cancel_flag);
	
	if(ret==ASD_WLAN_NOT_EXIST)
		vty_out(vty,"<error> wlan doesn't work.\n");	/*xm0723*/
	else if(ret == ASD_STA_NOT_EXIST)
		vty_out(vty,"<error> can't find sta under wlan %u\n",wlan_id);
	else if(ret != ASD_DBUS_SUCCESS)
		vty_out(vty,"<error> other error %d(asd)\n",ret);
	dbus_message_unref(reply);


	if(ret == ASD_DBUS_SUCCESS)
	{
		
	/*	int index = 0;
		char BUSNAME[PATH_LEN];
		char OBJPATH[PATH_LEN];
		char INTERFACE[PATH_LEN];
		if(vty->node == RADIO_NODE){
			index = 0;			
			RadioID = (int)vty->index;
		}else if(vty->node == HANSI_RADIO_NODE){
			index = vty->index; 		
			RadioID = (int)vty->index_sub;
		}*/
		memset(BUSNAME,0,PATH_LEN);
		memset(OBJPATH,0,PATH_LEN);
		memset(INTERFACE,0,PATH_LEN);

		ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
		ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
		ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_CANCEL_STA_VALUE);
		
	/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_CANCEL_STA_VALUE);*/
		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&RadioID,
								DBUS_TYPE_BYTE,&wlan_id,
								DBUS_TYPE_BYTE,&cancel_flag,
								DBUS_TYPE_BYTE,&mac1[0],
								DBUS_TYPE_BYTE,&mac1[1],
								DBUS_TYPE_BYTE,&mac1[2],
								DBUS_TYPE_BYTE,&mac1[3],
								DBUS_TYPE_BYTE,&mac1[4],
								DBUS_TYPE_BYTE,&mac1[5],
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

		if(ret==WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan isn't existed\n");
		else if(ret == WTP_ID_NOT_EXIST)
			vty_out(vty,"<error> wtp isn't existed\n");
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio isn't existed\n");
		else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)  //fengwenchao add 20120428 for autelan-2905
			vty_out(vty,"<error> radio doesn't bind wlan %s\n",argv[0]);
		else if(ret == WID_DBUS_SUCCESS)
			vty_out(vty,"set wlan %s traffic limit cancel station %s value successfully!\n",argv[0],argv[1]);
		else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
		{
			vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
		}
		else
			vty_out(vty,"<error> other error %d(wid)\n",ret);
		dbus_message_unref(reply);
	}

	return CMD_SUCCESS; 
}
#endif
#if _GROUP_POLICY
/*traffic limit send area*/
DEFUN(radio_bss_taffic_limit_send_value_func,
		radio_bss_taffic_limit_send_value_cmd,
		"wlan ID traffic limit send value VALUE",
		"wlan configure\n"
		"wlan id\n" 
		"traffic limit\n" 
		"traffic limit\n"
		"traffic limit send value \n"
		"traffic limit send value \n"
		"traffic limit send value (kbps)\n"
		)
{ 	
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned char wlan_id = 0;
	unsigned int value = 0;  

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 		

	wlan_id = atoi(argv[0]);
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	ret = parse_int_ID((char *)argv[1],&value);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[1]);
		return CMD_SUCCESS;
	} 
	if((value > 884736)||(value < 1))      /*fengwenchao add 20110228  108*1024*8 = 884736*/
	{
		vty_out(vty,"<error> input value should be 1 to 884736\n");
		return CMD_SUCCESS;
	}	
	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index;
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
    } 
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = radio_bss_taffic_limit_send_value_cmd_wlan_ID_traffic_limit_send_value(localid,index,dcli_dbus_connection,type,id,wlan_id,value,
	&count,&ret);

	if(type==0)
		{
			if(ret == WID_DBUS_SUCCESS)		
				vty_out(vty,"set wlan %s traffic limit send value %s successfully!\n",argv[0],argv[1]);
			else if(ret == WTP_ID_NOT_EXIST)
				vty_out(vty,"<error> wtp isn't existed\n");
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio isn't existed\n");
			else if(ret==RADIO_NO_BINDING_WLAN)
				vty_out(vty,"<error> radio doesn't bind wlan %s\n",argv[0]);
			else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
				vty_out(vty,"<error> radio doesn't bind wlan\n");
			else if(ret==WLAN_ID_NOT_EXIST)
				vty_out(vty,"<error> wlan isn't existed\n");
			else
				vty_out(vty,"<error> other error\n");
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set wlan %s traffic limit send value %s successfully!\n",id,argv[0],argv[1]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}
	
	return CMD_SUCCESS; 
}

#else
/*traffic limit send area*/
DEFUN(radio_bss_taffic_limit_send_value_func,
		radio_bss_taffic_limit_send_value_cmd,
		"wlan ID traffic limit send value VALUE",
		"wlan configure\n"
		"wlan id\n" 
		"traffic limit\n" 
		"traffic limit\n"
		"traffic limit send value \n"
		"traffic limit send value \n"
		"traffic limit send value (kbps)\n"
		)
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
	unsigned int RadioID;
	unsigned char wlan_id = 0;
	unsigned int value = 0;   
	
	//RadioID = (int)vty->index;
	wlan_id = atoi(argv[0]);
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	ret = parse_int_ID((char *)argv[1],&value);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[1]);
		return CMD_SUCCESS;
	} 
	if((value > 884736)||(value < 1))      /*fengwenchao add 20110228  108*1024*8 = 884736*/
	{
		vty_out(vty,"<error> input value should be 1 to 884736\n");
		return CMD_SUCCESS;
	}	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		RadioID = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		RadioID = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		RadioID = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_SEND_VALUE);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_SEND_VALUE);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&RadioID,
							DBUS_TYPE_BYTE,&wlan_id,
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
	
	if(ret == WID_DBUS_SUCCESS)		
		vty_out(vty,"set wlan %s traffic limit send value %s successfully!\n",argv[0],argv[1]);
	else if(ret == WTP_ID_NOT_EXIST)
		vty_out(vty,"<error> wtp isn't existed\n");
	else if(ret == RADIO_ID_NOT_EXIST)
		vty_out(vty,"<error> radio isn't existed\n");
	else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
		vty_out(vty,"<error> radio doesn't bind wlan %s\n",argv[0]);
	else if(ret==WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan isn't existed\n");
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error> other error\n");
	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}
#endif
#if _GROUP_POLICY
DEFUN(radio_bss_taffic_limit_average_send_value_func,
		radio_bss_taffic_limit_average_send_value_cmd,
		"wlan ID traffic limit station average send value VALUE",
		"wlan configure\n"
		"wlan id\n" 
		"traffic limit\n" 
		"traffic limit\n"
		"traffic limit station send value \n"
		"traffic limit station send average value \n"
		"traffic limit station send average value\n"
		"traffic limit station send average value \n"
		"traffic limit station average value (kbps)\n"
		)
{ 
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned char wlan_id = 0;
	unsigned int value = 0;  

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 		
	wlan_id = atoi(argv[0]);
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	ret = parse_int_ID((char *)argv[1],&value);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[1]);
		return CMD_SUCCESS;
	} 
	if((value > 884736)||(value < 1))      /*fengwenchao add 20110228  108*1024*8 = 884736*/
	{
		vty_out(vty,"<error> input value should be 1 to 884736\n");
		return CMD_SUCCESS;
	}	
	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    } 
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = radio_bss_taffic_limit_average_send_value_cmd_wlan_configure(localid,index,dcli_dbus_connection,type,id,wlan_id,value,
	&count,&ret);

	if(type==0)
		{
			if(ret == IF_POLICY_CONFLICT)
				vty_out(vty,"<error> station traffic limit value is more than bss traffic limit value\n");
			else if(ret == WTP_ID_NOT_EXIST)
				vty_out(vty,"<error> wtp isn't existed\n");
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio isn't existed\n");
			else if(ret==RADIO_NO_BINDING_WLAN)
				vty_out(vty,"<error> radio doesn't bind wlan %s\n",argv[0]);
			else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
				vty_out(vty,"<error> radio doesn't bind wlan\n");
			else if(ret == WID_DBUS_SUCCESS)
				vty_out(vty,"set wlan %s traffic limit station average value %s successfully!\n",argv[0],argv[1]);
			else if(ret==WLAN_ID_NOT_EXIST)
				vty_out(vty,"<error> wlan isn't existed\n");
			else
				vty_out(vty,"<error> other error\n");
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set wlan %s traffic limit station average value %s successfully!\n",id,argv[0],argv[1]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}	


	return CMD_SUCCESS; 
}

#else
DEFUN(radio_bss_taffic_limit_average_send_value_func,
		radio_bss_taffic_limit_average_send_value_cmd,
		"wlan ID traffic limit station average send value VALUE",
		"wlan configure\n"
		"wlan id\n" 
		"traffic limit\n" 
		"traffic limit\n"
		"traffic limit station send value \n"
		"traffic limit station send average value \n"
		"traffic limit station send average value\n"
		"traffic limit station send average value \n"
		"traffic limit station average value (kbps)\n"
		)
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
	unsigned int RadioID;
	unsigned char wlan_id = 0;
	unsigned int value = 0;   
	
	//RadioID = (int)vty->index;
	wlan_id = atoi(argv[0]);
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	ret = parse_int_ID((char *)argv[1],&value);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[1]);
		return CMD_SUCCESS;
	} 
	if((value > 884736)||(value < 1))      /*fengwenchao add 20110228  108*1024*8 = 884736*/
	{
		vty_out(vty,"<error> input value should be 1 to 884736\n");
		return CMD_SUCCESS;
	}	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		RadioID = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		RadioID = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		RadioID = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_AVERAGE_SEND_VALUE);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_AVERAGE_SEND_VALUE);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&RadioID,
							DBUS_TYPE_BYTE,&wlan_id,
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
	
	if(ret == IF_POLICY_CONFLICT)
		vty_out(vty,"<error> station traffic limit value is more than bss traffic limit value\n");
	else if(ret == WTP_ID_NOT_EXIST)
		vty_out(vty,"<error> wtp isn't existed\n");
	else if(ret == RADIO_ID_NOT_EXIST)
		vty_out(vty,"<error> radio isn't existed\n");
	else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)  //fengwenchao add 20120428 for autelan-2905
		vty_out(vty,"<error> radio doesn't bind wlan %s\n",argv[0]);
	else if(ret == WID_DBUS_SUCCESS)
		vty_out(vty,"set wlan %s traffic limit station average value %s successfully!\n",argv[0],argv[1]);
	else if(ret==WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan isn't existed\n");
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error> other error\n");
	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}
#endif
#if _GROUP_POLICY
DEFUN(radio_bss_taffic_limit_sta_send_value_func,
		radio_bss_taffic_limit_sta_send_value_cmd,
		"wlan ID traffic limit station MAC send value VALUE",
		"traffic limit station MAC xx:xx:xx:xx:xx:xx\n"
		"traffic limit station send value (kbps)\n"
		)
{	
	 

	int i = 0;
	int ret = 0;
	int ret1 = 0;
	int ret2 = 0;
	int ret3 = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	int failnum = 0;
	unsigned int id = 0;
	unsigned int TYPE = 0;
	unsigned char wlan_id = 0;
	unsigned char	type = 2;
	unsigned int	value = 0;
	unsigned int	mac[MAC_LEN]={0};
	unsigned char	mac1[MAC_LEN]={0};
	
	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
	
	wlan_id = atoi(argv[0]);
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	memset(mac,0,MAC_LEN);
	sscanf(argv[1],"%X:%X:%X:%X:%X:%X",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
	mac1[0] = (unsigned char)mac[0];
	mac1[1] = (unsigned char)mac[1];	
	mac1[2] = (unsigned char)mac[2];	
	mac1[3] = (unsigned char)mac[3];	
	mac1[4] = (unsigned char)mac[4];	
	mac1[5] = (unsigned char)mac[5];

	ret = parse_int_ID((char *)argv[2],&value);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[2]);
		return CMD_SUCCESS;
	} 
	if((value > 884736)||(value < 1))      /*fengwenchao add 20110228  108*1024*8 = 884736*/
	{
		vty_out(vty,"<error> input value should be 1 to 884736\n");
		return CMD_SUCCESS;
	}	
	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		TYPE = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		TYPE= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		TYPE= 1;
    } 
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = radio_bss_taffic_limit_sta_send_value_cmd_wlan_traffic_limit_station_send_value(localid,index,dcli_dbus_connection,
		TYPE,id,mac1,wlan_id,type,value,&count,&ret,&ret1,&ret2,&ret3,&failnum);

	vty_out(vty,"ret = %d,ret2 = %d,ret3 = %d\n",ret,ret2,ret3);
	
	if(TYPE==0)
		{
			if(ret == 0)
				{	
					if(ret2 == 0)
						{
							if(ret3 == 0)
								{
									vty_out(vty,"set static info successfully.\n");
								}
							else if (ret3 == ASD_DBUS_ERROR)
									vty_out(vty,"<error> set info failed get reply.\n");
							else
									vty_out(vty,"<error> set info ret = %d.\n",ret3);
						}
					else if (ret2 == ASD_DBUS_ERROR)
							vty_out(vty,"<error> wid set failed get reply.\n");
					else
							vty_out(vty,"<error> wid set error ret = %d\n",ret2);
				}
			else if(ret==ASD_WLAN_NOT_EXIST)
					vty_out(vty,"<error> wlan doesn't work.\n");
			else if(ret == ASD_STA_NOT_EXIST)
					vty_out(vty,"<error> can't find sta under wlan %u.\n",wlan_id);
			else if(ret == ASD_DBUS_ERROR)
					vty_out(vty,"<error> value great than bss traffic limit.\n");
			else
					vty_out(vty,"<error> asd set ret = %d.\n",ret);
		}
	else if(TYPE==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set static info successfully.\n",id);
					if((count != 0)&&(TYPE == 1)&&(RadioList_Head!=NULL)&&(failnum != 0))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<failnum; i++)
								{
	
									vty_out(vty,"%d ",RadioList_Head[i].RadioId);
									//vty_out(vty,"%d",RadioList_Head[i].FailReason);
								}
							vty_out(vty," failed.\n");
							if(RadioList_Head != NULL)
								{
									free(RadioList_Head);
									RadioList_Head =NULL;
								}
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}	
	return CMD_SUCCESS; 
}

#else
DEFUN(radio_bss_taffic_limit_sta_send_value_func,
		radio_bss_taffic_limit_sta_send_value_cmd,
		"wlan ID traffic limit station MAC send value VALUE",
		"traffic limit station MAC xx:xx:xx:xx:xx:xx\n"
		"traffic limit station send value (kbps)\n"
		)
{	
	 
	unsigned int	value = 0;
	unsigned int	mac[MAC_LEN]={0};
	unsigned char	mac1[MAC_LEN]={0};
	unsigned int	ret=0;
	unsigned int 	RadioID = 0;
	unsigned char 	wlan_id = 0;
	unsigned char	type = 2;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	
	wlan_id = atoi(argv[0]);
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	memset(mac,0,MAC_LEN);
	sscanf(argv[1],"%X:%X:%X:%X:%X:%X",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
	mac1[0] = (unsigned char)mac[0];
	mac1[1] = (unsigned char)mac[1];	
	mac1[2] = (unsigned char)mac[2];	
	mac1[3] = (unsigned char)mac[3];	
	mac1[4] = (unsigned char)mac[4];	
	mac1[5] = (unsigned char)mac[5];

	ret = parse_int_ID((char *)argv[2],&value);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[2]);
		return CMD_SUCCESS;
	} 
	if((value > 884736)||(value < 1))      /*fengwenchao add 20110228  108*1024*8 = 884736*/
	{
		vty_out(vty,"<error> input value should be 1 to 884736\n");
		return CMD_SUCCESS;
	}	
	if(vty->node == RADIO_NODE){
		index = 0;			
		RadioID = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		RadioID = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		RadioID = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	asd_set_sta_info(dcli_dbus_connection,index,mac1,wlan_id,RadioID,type,value,localid,&ret);

	if(ret == 0){
		wid_set_sta_info(dcli_dbus_connection,index,mac1,wlan_id,RadioID,type,value,localid,&ret);
		if(ret == 0){
			set_sta_static_info(dcli_dbus_connection,index,mac1,wlan_id,RadioID,type,value,localid,&ret);
			if(ret == 0){
				vty_out(vty,"set static info successfully.\n");
			}else if (ret == ASD_DBUS_ERROR)
				vty_out(vty,"<error> set info failed get reply.\n");
			else
				vty_out(vty,"<error> set info ret = %d.\n",ret);
		}else if (ret == ASD_DBUS_ERROR)
			vty_out(vty,"<error> wid set failed get reply.\n");
		else
			vty_out(vty,"<error> wid set error ret = %d\n",ret);
	}
	else if(ret==ASD_WLAN_NOT_EXIST)
		vty_out(vty,"<error> wlan doesn't work.\n");
	else if(ret == ASD_STA_NOT_EXIST)
		vty_out(vty,"<error> can't find sta under wlan %u.\n",wlan_id);
	else if(ret == ASD_DBUS_ERROR)
		vty_out(vty,"<error> value great than bss traffic limit.\n");
	else
		vty_out(vty,"<error> asd set ret = %d.\n",ret);

	return CMD_SUCCESS; 
}
#endif
#if 0
DEFUN(radio_bss_taffic_limit_sta_send_value_func,
		radio_bss_taffic_limit_sta_send_value_cmd,
		"wlan ID traffic limit station MAC send value VALUE",
		"wlan configure\n"
		"wlan id\n" 
		"traffic limit\n" 
		"traffic limit\n"
		"traffic limit station send value \n"
		"traffic limit station MAC xx:xx:xx:xx:xx:xx\n"
		"traffic limit station send value \n"
		"traffic limit station send value \n"
		"traffic limit station send value (kbps)\n"
		)
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
	unsigned int RadioID;
	unsigned char wlan_id = 0;
	unsigned int value = 0;   
	unsigned char mac1[MAC_LEN];
	unsigned int mac[MAC_LEN];
	
	//RadioID = (int)vty->index;
	wlan_id = atoi(argv[0]);
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	memset(mac,0,MAC_LEN);
	sscanf(argv[1],"%X:%X:%X:%X:%X:%X",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
	mac1[0] = (unsigned char)mac[0];
	mac1[1] = (unsigned char)mac[1];	
	mac1[2] = (unsigned char)mac[2];	
	mac1[3] = (unsigned char)mac[3];	
	mac1[4] = (unsigned char)mac[4];	
	mac1[5] = (unsigned char)mac[5];


	
	ret = parse_int_ID((char *)argv[2],&value);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[2]);
		return CMD_SUCCESS;
	} 
	
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		RadioID = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		RadioID = (int)vty->index_sub;
	}
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_STA_SEND_TRAFFIC_LIMIT);

/*	query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_STA_SEND_TRAFFIC_LIMIT);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&RadioID,
							DBUS_TYPE_BYTE,&wlan_id,
							DBUS_TYPE_UINT32,&value,
							DBUS_TYPE_BYTE,&mac1[0],
							DBUS_TYPE_BYTE,&mac1[1],
							DBUS_TYPE_BYTE,&mac1[2],
							DBUS_TYPE_BYTE,&mac1[3],
							DBUS_TYPE_BYTE,&mac1[4],
							DBUS_TYPE_BYTE,&mac1[5],
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
	
	if(ret==ASD_WLAN_NOT_EXIST)
		vty_out(vty,"<error> wlan doesn't work.\n");
	else if(ret == ASD_STA_NOT_EXIST)
		vty_out(vty,"<error> can't find sta under wlan %u\n",wlan_id);
	else if(ret == ASD_DBUS_ERROR)
		vty_out(vty,"<error> value great than bss send traffic limit .\n");
	dbus_message_unref(reply);


	if(ret == ASD_DBUS_SUCCESS)
	{
		
	/*	int index = 0;
		char BUSNAME[PATH_LEN];
		char OBJPATH[PATH_LEN];
		char INTERFACE[PATH_LEN];
		if(vty->node == RADIO_NODE){
			index = 0;			
			RadioID = (int)vty->index;
		}else if(vty->node == HANSI_RADIO_NODE){
			index = vty->index; 		
			RadioID = (int)vty->index_sub;
		}*/
		memset(BUSNAME,0,PATH_LEN);
		memset(OBJPATH,0,PATH_LEN);
		memset(INTERFACE,0,PATH_LEN);

		ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
		ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
		ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_STA_SEND_VALUE);
		
	/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_STA_SEND_VALUE);*/
		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_BYTE,&wlan_id,
								DBUS_TYPE_UINT32,&RadioID,
								DBUS_TYPE_UINT32,&value,
								DBUS_TYPE_BYTE,&mac1[0],
								DBUS_TYPE_BYTE,&mac1[1],
								DBUS_TYPE_BYTE,&mac1[2],
								DBUS_TYPE_BYTE,&mac1[3],
								DBUS_TYPE_BYTE,&mac1[4],
								DBUS_TYPE_BYTE,&mac1[5],
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
			vty_out(vty,"set wlan %s traffic limit station %s send value %s successfully!\n",argv[0],argv[1],argv[2]);
		else if(ret == WTP_ID_NOT_EXIST)
			vty_out(vty,"<error> wtp isn't existed\n");
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio isn't existed\n");
		else if(ret == IF_POLICY_CONFLICT)
			vty_out(vty,"<error> station traffic limit send value is more than bss traffic limit send value\n");
		else if(ret==RADIO_NO_BINDING_WLAN)
			vty_out(vty,"<error> radio doesn't bind wlan %s\n",argv[0]);
		else if(ret==WLAN_ID_NOT_EXIST)
			vty_out(vty,"<error> wlan isn't existed\n");
		else
			vty_out(vty,"<error> other error\n");
		dbus_message_unref(reply);
	}

	return CMD_SUCCESS; 
}
#endif

#if _GROUP_POLICY
DEFUN(radio_bss_taffic_limit_cancel_sta_send_value_func,
		radio_bss_taffic_limit_cancel_sta_send_value_cmd,
		"wlan ID traffic limit cancel station MAC send value",
		"wlan configure\n"
		"wlan id\n" 
		"traffic limit\n" 
		"traffic limit\n"
		"traffic limit cancel station send value \n"
		"traffic limit cancel station MAC xx:xx:xx:xx:xx:xx\n"
		"traffic limit cancel station send value \n"
		"traffic limit cancel station send value \n"
		)
{
	int i = 0;
	int ret = 0;
	int ret_asd = 0;
	int ret_wid = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	int failnum = 0;
	unsigned int id = 0;
	unsigned int type = 0;
  	unsigned char wlan_id = 0;
	unsigned char cancel_flag = 0;
	unsigned char mac1[MAC_LEN];
	unsigned int mac[MAC_LEN];

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 		

	wlan_id = atoi(argv[0]);
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	memset(mac,0,MAC_LEN);
	sscanf(argv[1],"%X:%X:%X:%X:%X:%X",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
	mac1[0] = (unsigned char)mac[0];
	mac1[1] = (unsigned char)mac[1];	
	mac1[2] = (unsigned char)mac[2];	
	mac1[3] = (unsigned char)mac[3];	
	mac1[4] = (unsigned char)mac[4];	
	mac1[5] = (unsigned char)mac[5];

	
	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    } 
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	RadioList_Head = radio_bss_taffic_limit_cancel_sta_send_value_cmd_wlan_configure(localid,index,dcli_dbus_connection,type,id,wlan_id,
	mac1,&count,&ret,&ret_asd,&ret_wid,&failnum);

	vty_out(vty,"ret = %d\n",ret);
	if(type==0)
		{
			if(ret == WID_DBUS_SUCCESS)
				{
					if(ret_asd == ASD_WLAN_NOT_EXIST)
						vty_out(vty,"<error> wlan doesn't work.\n");	
					else if(ret_asd == ASD_STA_NOT_EXIST)
						vty_out(vty,"<error> can't find sta under wlan %u\n",wlan_id);
					else if(ret_asd != ASD_DBUS_SUCCESS)
						vty_out(vty,"<error> other error %d(asd)\n",ret_asd);
					if(ret_asd == ASD_DBUS_SUCCESS)
						{
							if(ret_wid == WID_DBUS_SUCCESS)
								vty_out(vty,"set wlan %s traffic limit cancel station %s send value successfully!\n",argv[0],argv[1]);
							else if(ret_wid == WTP_ID_NOT_EXIST)
								vty_out(vty,"<error> wtp isn't existed\n");
							else if(ret_wid == RADIO_ID_NOT_EXIST)
								vty_out(vty,"<error> radio isn't existed\n");
							else if(ret_wid==RADIO_NO_BINDING_WLAN)
								vty_out(vty,"<error> radio doesn't bind wlan %s\n",argv[0]);
							else if(ret_wid==WLAN_ID_NOT_EXIST)
								vty_out(vty,"<error> wlan isn't existed\n");
							else
								vty_out(vty,"<error> other error %d(wid)\n",ret);
						}
				}
			else
				{
					vty_out(vty,"<error> radioid is not exist.\n");
				}
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set wlan %s traffic limit cancel station %s send value successfully!\n",id,argv[0],argv[1]);
					if((failnum != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<failnum; i++)
								{
									vty_out(vty,"%d ",RadioList_Head[i].RadioId);	
									vty_out(vty,"%d ",RadioList_Head[i].FailReason);		
								}
							vty_out(vty," failed.\n");
							//dcli_free_RadioList(RadioList_Head);
							if(RadioList_Head !=NULL)
								{
									free(RadioList_Head);
									RadioList_Head = NULL;
								}
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");		
		}	

	return CMD_SUCCESS; 
}

#else
DEFUN(radio_bss_taffic_limit_cancel_sta_send_value_func,
		radio_bss_taffic_limit_cancel_sta_send_value_cmd,
		"wlan ID traffic limit cancel station MAC send value",
		"wlan configure\n"
		"wlan id\n" 
		"traffic limit\n" 
		"traffic limit\n"
		"traffic limit cancel station send value \n"
		"traffic limit cancel station MAC xx:xx:xx:xx:xx:xx\n"
		"traffic limit cancel station send value \n"
		"traffic limit cancel station send value \n"
		)
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
	unsigned int RadioID;
	unsigned char wlan_id = 0;
	unsigned char cancel_flag = 0;
	unsigned int value = 0;   
	unsigned char mac1[MAC_LEN];
	unsigned int mac[MAC_LEN];
	
	//RadioID = (int)vty->index;
	wlan_id = atoi(argv[0]);
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	memset(mac,0,MAC_LEN);
	sscanf(argv[1],"%X:%X:%X:%X:%X:%X",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
	mac1[0] = (unsigned char)mac[0];
	mac1[1] = (unsigned char)mac[1];	
	mac1[2] = (unsigned char)mac[2];	
	mac1[3] = (unsigned char)mac[3];	
	mac1[4] = (unsigned char)mac[4];	
	mac1[5] = (unsigned char)mac[5];

	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		RadioID = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		RadioID = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		RadioID = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_STA_SEND_TRAFFIC_LIMIT_CANCEL);
	
/*	query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_STA_SEND_TRAFFIC_LIMIT_CANCEL);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&RadioID,
							DBUS_TYPE_BYTE,&wlan_id,
							DBUS_TYPE_BYTE,&mac1[0],
							DBUS_TYPE_BYTE,&mac1[1],
							DBUS_TYPE_BYTE,&mac1[2],
							DBUS_TYPE_BYTE,&mac1[3],
							DBUS_TYPE_BYTE,&mac1[4],
							DBUS_TYPE_BYTE,&mac1[5],
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
	dbus_message_iter_get_basic(&iter,&cancel_flag);
	
	if(ret==ASD_WLAN_NOT_EXIST)
		vty_out(vty,"<error> wlan doesn't work.\n");	/*xm0723*/
	else if(ret == ASD_STA_NOT_EXIST)
		vty_out(vty,"<error> can't find sta under wlan %u\n",wlan_id);
	else if(ret != ASD_DBUS_SUCCESS)
		vty_out(vty,"<error> other error %d(asd)\n",ret);
	dbus_message_unref(reply);


	if(ret == ASD_DBUS_SUCCESS)
	{
		
	/*	int index = 0;
		char BUSNAME[PATH_LEN];
		char OBJPATH[PATH_LEN];
		char INTERFACE[PATH_LEN];
		if(vty->node == RADIO_NODE){
			index = 0;			
			RadioID = (int)vty->index;
		}else if(vty->node == HANSI_RADIO_NODE){
			index = vty->index; 		
			RadioID = (int)vty->index_sub;
		}*/
		memset(BUSNAME,0,PATH_LEN);
		memset(OBJPATH,0,PATH_LEN);
		memset(INTERFACE,0,PATH_LEN);

		ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
		ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
		ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_CANCEL_STA_SEND_VALUE);
		
	/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_CANCEL_STA_SEND_VALUE);*/
		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&RadioID,
								DBUS_TYPE_BYTE,&wlan_id,
								DBUS_TYPE_BYTE,&cancel_flag,
								DBUS_TYPE_BYTE,&mac1[0],
								DBUS_TYPE_BYTE,&mac1[1],
								DBUS_TYPE_BYTE,&mac1[2],
								DBUS_TYPE_BYTE,&mac1[3],
								DBUS_TYPE_BYTE,&mac1[4],
								DBUS_TYPE_BYTE,&mac1[5],
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
			vty_out(vty,"set wlan %s traffic limit cancel station %s send value successfully!\n",argv[0],argv[1]);
		else if(ret == WTP_ID_NOT_EXIST)
			vty_out(vty,"<error> wtp isn't existed\n");
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio isn't existed\n");
		else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)  //fengwenchao add 20120428 for autelan-2905
			vty_out(vty,"<error> radio doesn't bind wlan %s\n",argv[0]);
		else if(ret==WLAN_ID_NOT_EXIST)
			vty_out(vty,"<error> wlan isn't existed\n");
		else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
		{
			vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
		}
		else
			vty_out(vty,"<error> other error %d(wid)\n",ret);
		dbus_message_unref(reply);
	}

	return CMD_SUCCESS; 
}
#endif
#if _GROUP_POLICY
DEFUN(radio_apply_wlan_clean_vlan_cmd_func,
		radio_apply_wlan_clean_vlan_cmd,
		"radio apply wlan ID clean vlan",
		CONFIG_STR
		"radio binding information\n" 
		"radio binding wlan information\n" 
		"assign wlan id \n"
		"radio apply wlan clean vlan\n"
		"radio apply wlan clean vlan\n"
		)
{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned char wlan_id = 0;

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 	

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
	
	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    } 
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = radio_apply_wlan_clean_vlan_cmd_radio_apply_wlan_ID(localid,index,dcli_dbus_connection,type,id,wlan_id,
		&count,&ret);
	
	if(type==0)
		{
			if(ret == 0)
				vty_out(vty,"radio %d-%d binding wlan %d clean vlanid successfully.\n",id/L_RADIO_NUM,id%L_RADIO_NUM,wlan_id);
			else if(ret == WTP_ID_NOT_EXIST)
				vty_out(vty,"<error> wtp id does not exist\n");
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio id does not exist\n");
			else if(ret == WLAN_ID_NOT_EXIST)
				vty_out(vty,"<error> binding wlan does not exist \n");
			else if(ret == WTP_WLAN_BINDING_NOT_MATCH)
				vty_out(vty,"<error> radio is not binding this wlan \n");
			else if(ret == BSS_BE_ENABLE)
				vty_out(vty,"<error> bss is enable, you should disable it first\n");
			else
				vty_out(vty,"<error>  %d\n",ret);	
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d binding wlan %d clean vlanid successfully.\n",id,wlan_id);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}	
	return CMD_SUCCESS;	

}

#else
DEFUN(radio_apply_wlan_clean_vlan_cmd_func,
		radio_apply_wlan_clean_vlan_cmd,
		"radio apply wlan ID clean vlan",
		CONFIG_STR
		"radio binding information\n" 
		"radio binding wlan information\n" 
		"assign wlan id \n"
		"radio apply wlan clean vlan\n"
		"radio apply wlan clean vlan\n"
		)
{
	int ret;
	unsigned int radio_id;
	unsigned char wlan_id;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	//radio_id = (unsigned int)vty->index;	

	
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
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_RADIO_APPLY_WLANID_CLEAN_VLANID);
	
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_RADIO_APPLY_WLANID_CLEAN_VLANID);*/

	dbus_error_init(&err);

	dbus_message_append_args(query,					
						DBUS_TYPE_UINT32,&radio_id,
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

		if(ret == 0)
			vty_out(vty,"radio %d-%d binding wlan %d clean vlanid successfully.\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,wlan_id);
		else if(ret == WTP_ID_NOT_EXIST)
			vty_out(vty,"<error> wtp id does not exist\n");
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio id does not exist\n");
		else if(ret == WLAN_ID_NOT_EXIST)
			vty_out(vty,"<error> binding wlan does not exist \n");
		else if(ret == WTP_WLAN_BINDING_NOT_MATCH)
			vty_out(vty,"<error> radio is not binding this wlan \n");
		else if(ret == BSS_BE_ENABLE)
			vty_out(vty,"<error> bss is enable, you should disable it first\n");
		else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
		{
			vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
		}
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);

	return CMD_SUCCESS;	

}
#endif

DEFUN(set_radio_cpe_channel_apply_wlan_clean_vlan_id_cmd_func,
		set_radio_cpe_channel_apply_wlan_clean_vlan_id_cmd,
		"radio cpe channel apply wlan ID clean vlan ID",
		CONFIG_STR
		"cpe channel\n"
		"cpe channel\n"
		"radio binding information\n" 
		"radio binding wlan information\n" 
		"assign wlan id \n"
		"radio apply wlan clean vlan\n"
		"radio apply wlan clean vlan\n"
		"vlan ID"
		)
{
	int ret;
	unsigned int radio_id;
	unsigned char wlan_id;
	unsigned int vlan_id;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	//radio_id = (unsigned int)vty->index;	

	
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

	ret = parse_int_ID((char*)argv[1], &vlan_id);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(vlan_id > VLANID_RANGE_MAX|| vlan_id == 0){
		vty_out(vty,"<error> vlan id should be 1 to %d\n",VLANID_RANGE_MAX);
		return CMD_SUCCESS;
	}
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_RADIO_CPE_CHANNEL_APPLY_WLANID_CLEAN_VLANID);
	
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_RADIO_APPLY_WLANID_CLEAN_VLANID);*/

	dbus_error_init(&err);

	dbus_message_append_args(query,					
						DBUS_TYPE_UINT32,&radio_id,
						DBUS_TYPE_BYTE,&wlan_id,
						DBUS_TYPE_UINT32,&vlan_id,
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
			vty_out(vty,"radio %d-%d cpe channel binding wlan %d clean vlanid successfully.\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,wlan_id);
		else if(ret == WTP_ID_NOT_EXIST)
			vty_out(vty,"<error> wtp id does not exist\n");
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio id does not exist\n");
		else if(ret == WLAN_ID_NOT_EXIST)
			vty_out(vty,"<error> binding wlan does not exist \n");
		else if(ret == WTP_WLAN_BINDING_NOT_MATCH)
			vty_out(vty,"<error> radio is not binding this wlan \n");
		else if(ret == BSS_BE_ENABLE)
			vty_out(vty,"<error> bss is enable, you should disable it first\n");
		else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
		{
			vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
		}
		else if(ret == VALUE_OUT_OF_RANGE)
		{
			vty_out(vty,"<error> radio binding more than 8 wlan\n");
		}
		else if (ret = BSS_NOT_EXIST)
		{
			vty_out(vty,"<error> radio cpe intf does not exists !\n");
		}
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);

	return CMD_SUCCESS;	

}


DEFUN(set_radio_cpe_channel_apply_wlan_clean_vlan_cmd_func,
		set_radio_cpe_channel_apply_wlan_clean_vlan_cmd,
		"radio cpe channel apply wlan ID clean vlan",
		CONFIG_STR
		"cpe channel\n"
		"cpe channel\n"
		"radio binding information\n" 
		"radio binding wlan information\n" 
		"assign wlan id \n"
		"radio apply wlan clean vlan\n"
		"radio apply wlan clean vlan\n"
		)
{
	int ret;
	unsigned int radio_id;
	unsigned char wlan_id;
	unsigned int vlan_id = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	//radio_id = (unsigned int)vty->index;	

	
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
	
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_RADIO_CPE_CHANNEL_APPLY_WLANID_CLEAN_VLANID);
	
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_RADIO_APPLY_WLANID_CLEAN_VLANID);*/

	dbus_error_init(&err);

	dbus_message_append_args(query,					
						DBUS_TYPE_UINT32,&radio_id,
						DBUS_TYPE_BYTE,&wlan_id,
						DBUS_TYPE_UINT32,&vlan_id,
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
			vty_out(vty,"radio %d-%d cpe channel binding wlan %d clean vlanid successfully.\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,wlan_id);
		else if(ret == WTP_ID_NOT_EXIST)
			vty_out(vty,"<error> wtp id does not exist\n");
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio id does not exist\n");
		else if(ret == WLAN_ID_NOT_EXIST)
			vty_out(vty,"<error> binding wlan does not exist \n");
		else if(ret == WTP_WLAN_BINDING_NOT_MATCH)
			vty_out(vty,"<error> radio is not binding this wlan \n");
		else if(ret == BSS_BE_ENABLE)
			vty_out(vty,"<error> bss is enable, you should disable it first\n");
		else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
		{
			vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
		}
		else if(ret == VALUE_OUT_OF_RANGE)
		{
			vty_out(vty,"<error> radio binding more than 8 wlan\n");
		}
		else if (ret = BSS_NOT_EXIST)
		{
			vty_out(vty,"<error> radio cpe intf does not exists !\n");
		}
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);

	return CMD_SUCCESS;	

}


DEFUN(radio_rx_data_dead_time_cmd_func,
		radio_rx_data_dead_time_cmd,
		"set radio receive data deadtime VALUE",
		CONFIG_STR
		"radio dead time information\n" 
		"radio dead time information\n" 
		"dead time \n"
		"radio receive data dead time\n"
		"radio receive data dead time\n"
		)
{
	int ret;
	unsigned int radio_id = 0;
	unsigned int dead_time;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	//radio_id = (unsigned int)vty->index;	

	
	ret = parse_int_ID((char*)argv[0], &dead_time);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(dead_time > 1000 || dead_time == 0){
		vty_out(vty,"<error> dead time should be 1 to 1000 ms\n");
		return CMD_SUCCESS;
	}

	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_RADIO_RX_DATA_DEAD_TIME);


	/*
	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_SET_RADIO_RX_DATA_DEAD_TIME);*/

	dbus_error_init(&err);

	dbus_message_append_args(query,					
						DBUS_TYPE_UINT32,&radio_id,
						DBUS_TYPE_UINT32,&dead_time,	
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
			vty_out(vty,"radio %d-%d set radio receive data deadtime %d successfully.\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,dead_time);
		else if(ret == WTP_ID_NOT_EXIST)
			vty_out(vty,"<error> wtp id does not exist\n");
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio id does not exist\n");
		else if(ret == WLAN_ID_NOT_EXIST)
			vty_out(vty,"<error> binding wlan does not exist \n");
		else if(ret == BSS_BE_ENABLE)
			vty_out(vty,"<error> bss is enable, you should disable it first\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);

	return CMD_SUCCESS;	

}

/*wcl add for RDIR-33*/
DEFUN(set_radio_acktimeout_distance_cmd_func,
		set_radio_acktimeout_distance_cmd,
	  "acktimeout radio set distance DISTANCE",
	  "fixed format to input acktimeout\n"
	  "fixed format to input radio\n"
	  "fixed format to input set\n"
	  "fixed format to input distance\n"
	  "should be 0~40000\n"
	 )


{
	unsigned int radio_id; 
	unsigned int distance ;
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;	
	
	ret = parse_int_ID((char *)argv[0],&distance);
	if(distance < 0 || distance > 40000){
		vty_out(vty,"<error> distance should be 0~40000\n");
		return CMD_SUCCESS;
	}
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}

	int index = 0;
	int localid = 1;
    	int slot_id = HostSlotId;	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
        		localid = vty->local;
        		slot_id = vty->slotindex;				
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
	        index = vty->index;
	        localid = vty->local;
	        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_ACKTIMEOUT_DISTANCE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_UINT32,&distance,
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

		if(ret == 0)
			vty_out(vty,"Radio %d-%d set distance %d successfully .\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,distance);
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio id does not exist\n");
		else if(ret == RADIO_IS_DISABLE)
			vty_out(vty,"<error> radio is disable, please enable it first\n");
		else if(ret == WTP_IS_NOT_BINDING_WLAN_ID)
			vty_out(vty,"<error> radio is not binging wlan\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;

}
/*wcl add end*/
#if 1
DEFUN(set_sta_mac_vlanid_func,
		set_sta_mac_vlanid_cmd,
		"set sta MAC vlanid ID",
		"sta configure\n"
		"station MAC xx:xx:xx:xx:xx:xx\n"
		)
{	
	 
	struct dcli_sta_info *sta = NULL;
	unsigned int	value = 0;
	unsigned int	mac[MAC_LEN]={0};
	unsigned char	mac1[MAC_LEN]={0};
	unsigned char	type = 0;
	unsigned int	ret=0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	
	sscanf(argv[0],"%X:%X:%X:%X:%X:%X",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
	mac1[0] = (unsigned char)mac[0];
	mac1[1] = (unsigned char)mac[1];	
	mac1[2] = (unsigned char)mac[2];	
	mac1[3] = (unsigned char)mac[3];	
	mac1[4] = (unsigned char)mac[4];	
	mac1[5] = (unsigned char)mac[5];	

	ret = parse2_int_ID((char *)argv[1],&value);
	
	if(value > 4095 || value < 0){
		vty_out(vty,"<error> vlan id should be 0 to %d.\n",4095);
		return CMD_SUCCESS;
	}
	//mahz add 2011.5.31
	char *endptr = NULL;
	char c = (char *)argv[1][0];
	if(c == '0'){
		value = strtoul((char *)argv[1],&endptr,10);
		if(endptr[0] == '\0')
			ret =  ASD_DBUS_SUCCESS;
		else
			ret = ASD_UNKNOWN_ID;
	}//

	if (ret != ASD_DBUS_SUCCESS) {	
		vty_out(vty,"<error> input parameter %s error\n",argv[2]);
		return CMD_SUCCESS;
	} 

	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
        slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	sta = check_sta_by_mac(dcli_dbus_connection,index,mac1,type,value,localid,&ret);
	if((ret == 0) && (sta != NULL)){
		wid_set_sta_info(dcli_dbus_connection,index,mac1,sta->wlan_id,sta->radio_g_id,type,value,localid,&ret);
		if(ret == 0){
			set_sta_static_info(dcli_dbus_connection,index,mac1,sta->wlan_id,sta->radio_g_id,type,value,localid,&ret);
			if(ret == 0){
				vty_out(vty,"set static info successfully.\n");
			}else if (ret == ASD_DBUS_ERROR)
				vty_out(vty,"<error> set info failed get reply.\n");
			else
				vty_out(vty,"<error> set info ret = %d.\n",ret);
		}else
			vty_out(vty,"<error> wid set error ret = %d\n",ret);
		dcli_free_sta(sta);
	}else if (ret == ASD_STA_NOT_EXIST){
		vty_out(vty,"<error> can't find sta.\n");
	}else if (ret == ASD_DBUS_SET_ERROR){
		vty_out(vty,"<error> check sta set invalid value.\n");
	}else if (ret == ASD_DBUS_ERROR)
		vty_out(vty,"<error> check sta failed get reply.\n");
	else
		vty_out(vty,"<error> check sta error ret = %d.\n",ret);

	return CMD_SUCCESS;
}

#else
DEFUN(set_sta_mac_vlanid_func,
		set_sta_mac_vlanid_cmd,
		"set sta MAC vlanid ID",
		"sta configure\n"
		"station MAC xx:xx:xx:xx:xx:xx\n"
		)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	unsigned int 	ret;
	unsigned int 	vlan_id = 0; 
	unsigned char 	mac1[MAC_LEN];
	unsigned int 	mac[MAC_LEN];
	unsigned char 	wlan_id;
	unsigned int 	g_radioid;
	
	memset(mac,0,MAC_LEN);
	sscanf(argv[0],"%X:%X:%X:%X:%X:%X",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
	
	mac1[0] = (unsigned char)mac[0];
	mac1[1] = (unsigned char)mac[1];	
	mac1[2] = (unsigned char)mac[2];	
	mac1[3] = (unsigned char)mac[3];	
	mac1[4] = (unsigned char)mac[4];	
	mac1[5] = (unsigned char)mac[5];
	
	vlan_id = atoi((char*)argv[1]);
	if(vlan_id > 4095 || vlan_id < 0){
		vty_out(vty,"<error> vlan id should be 0 to %d.\n",4095);
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
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SET_STA_MAC_VLANID);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_SET_STA_MAC_VLANID);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&mac1[0],
							DBUS_TYPE_BYTE,&mac1[1],
							DBUS_TYPE_BYTE,&mac1[2],
							DBUS_TYPE_BYTE,&mac1[3],
							DBUS_TYPE_BYTE,&mac1[4],
							DBUS_TYPE_BYTE,&mac1[5],
							DBUS_TYPE_UINT32,&vlan_id,
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
	
	if(ret==ASD_STA_NOT_EXIST){
		vty_out(vty,"<error> can't find sta.\n");

	}else{
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&wlan_id);
	
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&g_radioid);
	
		dbus_message_unref(reply);
		memset(BUSNAME,0,PATH_LEN);
		memset(OBJPATH,0,PATH_LEN);
		memset(INTERFACE,0,PATH_LEN);
		ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
		ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
		ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_WLAN_SET_STA_VLANID);

		/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_WLAN_SET_STA_VLANID);*/
		dbus_error_init(&err);

		dbus_message_append_args(query,
									DBUS_TYPE_BYTE,&wlan_id,
									DBUS_TYPE_UINT32,&g_radioid,
									DBUS_TYPE_UINT32,&vlan_id,
									DBUS_TYPE_BYTE,&mac1[0],
									DBUS_TYPE_BYTE,&mac1[1],
									DBUS_TYPE_BYTE,&mac1[2],
									DBUS_TYPE_BYTE,&mac1[3],
									DBUS_TYPE_BYTE,&mac1[4],
									DBUS_TYPE_BYTE,&mac1[5],
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

		if(ret==WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan isn't existed.\n");
		else if(ret == WTP_ID_NOT_EXIST)
			vty_out(vty,"<error> wtp isn't existed.\n");
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio isn't existed.\n");
		else if(ret==RADIO_NO_BINDING_WLAN)
			vty_out(vty,"<error> radio doesn't bind wlan %s.\n",argv[0]);
		else if(ret == WID_DBUS_SUCCESS)
			vty_out(vty,"set sta vlanid successfully.\n");
		else
			vty_out(vty,"<error> other error %d.\n",ret);
		
		dbus_message_unref(reply);
	}

	return CMD_SUCCESS; 
}	
#endif

#if _GROUP_POLICY
/*ht add 091028*/
DEFUN(set_sta_dhcp_before_authorized_func,
	  set_sta_dhcp_before_authorized_cmd,
	  "wlan ID sta dhcp (enable|disable)",
	  "wlan configure\n"
	  "wlan id\n" 
	  "set sta get ip before authorized\n" 
	  "set wlanid ID sta dhcp enable|disable\n"
	 )
{
	int policy = 0;
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned char wlanid = 0;

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 	

	ret = parse_char_ID((char*)argv[0], &wlanid);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
			vty_out(vty,"<error> unknown id format\n");
			}
			return CMD_SUCCESS;
	}	
	if(wlanid >= WLAN_NUM || wlanid == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
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
	
	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    } 
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	RadioList_Head = set_sta_dhcp_before_authorized_cmd_wlan_ID_sta_dhcp(localid,index,dcli_dbus_connection,type,id,wlanid,policy,
	&count,&ret);

	if(type==0)
		{
			if(ret == 0)
				{
					vty_out(vty," set wlan %s sta dhcp %s successfully\n",argv[0],argv[1]);
				}
			else if (ret == WLAN_ID_NOT_EXIST)
				{
					vty_out(vty,"<error>  wlan %d not exist\n",wlanid);
				}
			else if (ret == WTP_IS_NOT_BINDING_WLAN_ID)
				{
					vty_out(vty,"<error>  wtp not binding wlan %d\n",wlanid);
				}
			else if (ret == WTP_NOT_IN_RUN_STATE)
					vty_out(vty,"<error> wtp id does not run\n");
			else if (ret == WTP_OVER_MAX_BSS_NUM)
				{
					vty_out(vty,"<error>  binding wlan error\n");
				}
			else
				{
					vty_out(vty,"<error>  %d\n",ret);
				}		
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set wlan %s sta dhcp %s successfully\n",id,argv[0],argv[1]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}			
	return CMD_SUCCESS; 		
}

#else
/*ht add 091028*/
DEFUN(set_sta_dhcp_before_authorized_func,
	  set_sta_dhcp_before_authorized_cmd,
	  "wlan ID sta dhcp (enable|disable)",
	  "wlan configure\n"
	  "wlan id\n" 
	  "set sta get ip before authorized\n" 
	  "set wlanid ID sta dhcp enable|disable\n"
	 )
{
	
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	unsigned int radioID = 0;
	unsigned char wlanid = 0;
	int ret = WID_DBUS_SUCCESS;
	//radioID = (unsigned int)vty->index;
	int policy = 0;

	ret = parse_char_ID((char*)argv[0], &wlanid);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
			vty_out(vty,"<error> unknown id format\n");
			}
			return CMD_SUCCESS;
	}	
	if(wlanid >= WLAN_NUM || wlanid == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
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
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radioID = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		radioID = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radioID = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_WLAN_SET_STA_DHCP_BEFORE_AUTHERIZED);
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_WLAN_SET_STA_DHCP_BEFORE_AUTHERIZED);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radioID,
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

	if(ret == 0)
	{
		vty_out(vty," set wlan %s sta dhcp %s successfully\n",argv[0],argv[1]);
	}
	else if (ret == WLAN_ID_NOT_EXIST)
	{
		vty_out(vty,"<error>  wlan %d not exist\n",wlanid);
	}
	else if (ret == WTP_IS_NOT_BINDING_WLAN_ID)
	{
		vty_out(vty,"<error>  wtp not binding wlan %d\n",wlanid);
	}
	else if (ret == WTP_NOT_IN_RUN_STATE)
		vty_out(vty,"<error> wtp id does not run\n");
	else if (ret == WTP_OVER_MAX_BSS_NUM)
	{
		vty_out(vty,"<error>  binding wlan error\n");
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
#endif
DEFUN(show_radio_receive_data_dead_time_cmd_func,
	  show_radio_receive_data_dead_time_cmd,
	  "show radio receive data dead time",
	  CONFIG_STR
	  "radio receive data dead time information\n"
	  "radio receive data dead time config\n"
	 )
{	int ret,i;
	ret = 0;
	unsigned int dead_time;
	unsigned int radioID;
	DBusMessage *query, *reply;
	DBusError err;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	
	//radioID = (int)vty->index;


	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radioID = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		radioID = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radioID = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_RADIO_RX_DATA_DEAD_TIME_SHOW);

	
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_SET_RADIO_RX_DATA_DEAD_TIME_SHOW);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radioID,
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
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&dead_time);
	
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"RADIOID %d-%d RECEIVE DATA DEAD TIME: %d\n",radioID/L_RADIO_NUM,radioID%L_RADIO_NUM, dead_time);
		vty_out(vty,"==============================================================================\n");
	}else if (ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan id does not exist\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
#if _GROUP_POLICY

/*ht add 091110*/
DEFUN(set_sta_ip_mac_binding_func,
	  set_sta_ip_mac_binding_cmd,
	  "wlan ID sta ip_mac binding (enable|disable)",
	  "radio configure\n"
	  "wlan id\n" 
	  "set sta ip_mac binding\n" 
	  "set wlanid ID sta ip_mac binding enable|disable\n"
	 )
{
	int policy = 0;
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned char wlanid = 0;

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 	

	ret = parse_char_ID((char*)argv[0], &wlanid);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
			vty_out(vty,"<error> unknown id format\n");
			}
			return CMD_SUCCESS;
	}	
	if(wlanid >= WLAN_NUM || wlanid == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
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
	
	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    } 
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = set_sta_ip_mac_binding_cmd_wlan_ID_sta_ip_mac_binding(localid,index,dcli_dbus_connection,type,id,wlanid,policy,
	&count,&ret);

	if(type==0)
		{
			if(ret == 0)
				{
					vty_out(vty," set wlan %s sta ip_mac binding %s successfully\n",argv[0],argv[1]);
				}
			else if (ret == WLAN_ID_NOT_EXIST)
				{
					vty_out(vty,"<error> wlan %d not exist\n",wlanid);
				}
			else if (ret == WTP_IS_NOT_BINDING_WLAN_ID)
				{
					vty_out(vty,"<error> wtp not binding wlan %d\n",wlanid);
				}
			else if (ret == WTP_OVER_MAX_BSS_NUM)
				{
					vty_out(vty,"<error> binding wlan error\n");
				}
			else
				{
					vty_out(vty,"<error> %d\n",ret);
				}
		}

	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set wlan %s sta ip_mac binding %s successfully\n",id,argv[0],argv[1]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}	

	return CMD_SUCCESS; 		
}

#else
/*ht add 091110*/
DEFUN(set_sta_ip_mac_binding_func,
	  set_sta_ip_mac_binding_cmd,
	  "wlan ID sta ip_mac binding (enable|disable)",
	  "radio configure\n"
	  "wlan id\n" 
	  "set sta ip_mac binding\n" 
	  "set wlanid ID sta ip_mac binding enable|disable\n"
	 )
{
	
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	unsigned int radioID = 0;
	unsigned char wlanid = 0;
	int ret = WID_DBUS_SUCCESS;
	//radioID = (unsigned int)vty->index;
	int policy = 0;

	ret = parse_char_ID((char*)argv[0], &wlanid);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
			vty_out(vty,"<error> unknown id format\n");
			}
			return CMD_SUCCESS;
	}	
	if(wlanid >= WLAN_NUM || wlanid == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
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
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radioID = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		radioID = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radioID = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_WLAN_SET_STA_IP_MAC_BINDING);
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
						WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_WLAN_SET_STA_IP_MAC_BINDING);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radioID,
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

	if(ret == 0)
	{
		vty_out(vty," set wlan %s sta ip_mac binding %s successfully\n",argv[0],argv[1]);
	}
	else if (ret == WLAN_ID_NOT_EXIST)
	{
		vty_out(vty,"<error> wlan %d not exist\n",wlanid);
	}
	else if (ret == WTP_IS_NOT_BINDING_WLAN_ID)
	{
		vty_out(vty,"<error> wtp not binding wlan %d\n",wlanid);
	}
	else if (ret == WTP_OVER_MAX_BSS_NUM)
	{
		vty_out(vty,"<error> binding wlan error\n");
	}
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
	{
		vty_out(vty,"<error> %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return CMD_SUCCESS; 		
}
#endif
#if _GROUP_POLICY

DEFUN(set_radio_guard_interval_cmd_func,
	  set_radio_guard_interval_cmd,
	  "11n guard interval (800|400)",
	  "set guard interval \n"
	  "Radio guard interval value 0:800ns 1:400ns \n"
	 )


{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned int interval = 0;
	unsigned short guard_interval = 0;

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 	

    /*rtsthre= atoi(argv[0]);*/
	ret = parse_int_ID((char *)argv[0],&interval);
	if(interval == 800){
		guard_interval = 0;

	}else if(interval == 400){
		guard_interval = 1;
	}
	//guard_interval =(unsigned short)interval;
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}

	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = set_radio_guard_interval_cmd_11n_guard_interval(localid,index,dcli_dbus_connection,type,id,guard_interval,
	&count,&ret);

	if(type==0)
		{
			if(ret == 0)
				vty_out(vty,"Radio %d-%d set guard interval %s successfully .\n",id/L_RADIO_NUM,id%L_RADIO_NUM,argv[0]);
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio id does not exist\n");
			else if(ret == RADIO_IS_DISABLE)
				vty_out(vty,"<error> radio is disable, please enable it first\n");
			else if(ret == WTP_IS_NOT_BINDING_WLAN_ID)
				vty_out(vty,"<error> radio is not binging wlan\n");
			else
				vty_out(vty,"<error>  %d\n",ret);	
		}

	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set guard interval %s successfully .\n",id,argv[0]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}	

	return CMD_SUCCESS;

}

#else
DEFUN(set_radio_guard_interval_cmd_func,
	  set_radio_guard_interval_cmd,
	  "11n guard interval (800|400)",
	  "set guard interval \n"
	  "Radio guard interval value 0:800ns 1:400ns \n"
	 )


{
	unsigned int radio_id; 
	unsigned int interval ;
	unsigned short guard_interval;
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;	
	
    /*rtsthre= atoi(argv[0]);*/
	ret = parse_int_ID((char *)argv[0],&interval);
	if(interval == 800){
		guard_interval = 0;

	}else if(interval == 400){
		guard_interval = 1;
	}
	//guard_interval =(unsigned short)interval;
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}

	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_GUARD_INTERVAL);

	//query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
		//				WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_SET_GUARD_INTERVAL);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_UINT16,&guard_interval,
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
			vty_out(vty,"Radio %d-%d set guard interval %s successfully .\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0]);
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio id does not exist\n");
		else if(ret == RADIO_IS_DISABLE)
			vty_out(vty,"<error> radio is disable, please enable it first\n");
		else if(ret == WTP_IS_NOT_BINDING_WLAN_ID)
			vty_out(vty,"<error> radio is not binging wlan\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;

}
#endif
#if _GROUP_POLICY
DEFUN(set_radio_mcs_cmd_func,
	  set_radio_mcs_cmd,
	  "11n mcs MCS",    //fengwenchao change <0-31> to MCS ,20110504
	  "set MCS \n"
	  "Radio MCS value\n"
	   "mcs <0-31>\n"                  //fengwenchao add 20110504
	 )

{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	//unsigned short mcs = 0;

	update_mcs_list *mcslist = NULL;  

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
	//qiuchen add
	struct tag_mcsid *tmp = NULL;
	mcslist = (struct tag_mcsid_list*)malloc(sizeof(struct tag_mcsid_list));
	mcslist->mcsidlist = NULL ; 	
	mcslist->count = 0;


	ret = parse_mcs_list((char*)argv[0],&mcslist);
	//printf("the mcs count is %d.\n",mcslist->count);
	printf("The mcs count is %d.\n",mcslist->count);//qiuchen add 
	if(ret != 0){
	    cli_syslog_info("parse mcs list failed\n");
	    return CMD_SUCCESS;
	}
	 	

	/*ret = parse_short_ID((char *)argv[0],&mcs);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
       if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
       }
	   else{
			   vty_out(vty,"<error> input parameter %s error\n",argv[0]);
	   }
		
		return CMD_SUCCESS;
	}

	if((mcs<0)||(mcs>31))
	{
       vty_out(vty,"<error> mcs should be 0 to 31\n");
		return CMD_SUCCESS;
	}*/

	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    } 	
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
   RadioList_Head = set_radio_mcs_cmd_11n_mcs(localid,index,dcli_dbus_connection,type,id,mcslist,&count,&ret);

	if(type==0)
		{
			if(ret == 0)
				vty_out(vty,"Radio %d-%d set mcs %s successfully .\n",id/L_RADIO_NUM,id%L_RADIO_NUM,argv[0]);
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio id does not exist\n");
			else if(ret == RADIO_IS_DISABLE)
				vty_out(vty,"<error> radio is disable, please enable it first\n");
			else if(ret == MCS_CROSS_THE_BORDER)                                  //fengwenchao add 20110408
				vty_out(vty,"<error> mcs cross the border, if your stream is one,mcs should be 0~7,if your stream is two,mcs should be 8~15,and if your stream is three,mcs should be 16~23\n");
			else
				vty_out(vty,"<error>  %d\n",ret);
			
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set mcs %s successfully .\n",id,argv[0]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}	

	return CMD_SUCCESS;

}

#else
DEFUN(set_radio_mcs_cmd_func,
	  set_radio_mcs_cmd,
	  "11n mcs MCS",                   //fengwenchao change <0-31> to MCS ,20110504
	  "set MCS \n"
	  "Radio MCS value\n"
	  "mcs <0-31>\n"                  //fengwenchao add 20110504
	 )


{
	update_mcs_list *mcslist = NULL;  int i =0;//fengwenchao add 20120314 for requiremnets-407
	unsigned int radio_id = 0; 
	unsigned short mcs;
	int ret = 0;
	DBusMessage *query = NULL, *reply = NULL;	
	DBusMessageIter	 iter;
	DBusError err;	
	
	/*fengwenchao add 20120314 for requirements-407*/
	struct tag_mcsid *tmp = NULL;
	mcslist = (struct tag_mcsid_list*)malloc(sizeof(struct tag_mcsid_list));
	mcslist->mcsidlist = NULL ; 	
	mcslist->count = 0;


	ret = parse_mcs_list((char*)argv[0],&mcslist);

	if(ret != 0){
	   // cli_syslog_info("parse mcs list failed\n");
	vty_out(vty,"<error> input mcs should be 0-31,format should be 1-31 or 1,2,31\n"); //fengwenchao add 20121214 for  AXSSZFI-1303
		free_mcs_list(mcslist);
	    return CMD_SUCCESS;
	}
	/*fengwenchao add end*/
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_MCS);
		

	//query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_RADIO_OBJPATH,\
	//					WID_DBUS_RADIO_INTERFACE,WID_DBUS_RADIO_METHOD_SET_MCS);
	
	dbus_error_init(&err);

	/*dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_UINT16,&mcs,
							 DBUS_TYPE_INVALID);*/
	/*fengwenchao copy from ht2.0 for requirements-407*/
	dbus_message_iter_init_append (query, &iter);
	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&radio_id);										 
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32,&(mcslist->count));	
										
	tmp = mcslist->mcsidlist;

	for(i = 0; ((i < mcslist->count) && (tmp)); i++){
		dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &(tmp->mcsid));
		tmp = tmp->next;
	}											 
	/*fengwenchao copy end*/
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		free_mcs_list(mcslist);
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

		if(ret == 0)
			vty_out(vty,"Radio %d-%d set MCS value: %s successfully.\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0]);
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio id does not exist\n");
		else if(ret == RADIO_IS_DISABLE)
			vty_out(vty,"<error> radio is disable, please enable it first\n");
		else if(ret == MCS_CROSS_THE_BORDER)                                  //fengwenchao add 20110408
			vty_out(vty,"<error> mcs cross the border, if your stream is one,mcs should be 0~7,if your stream is two,mcs should be 8~15,and if your stream is three,mcs should be 16~23\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	free_mcs_list(mcslist);	
	return CMD_SUCCESS;

}
#endif
#if _GROUP_POLICY
DEFUN(set_radio_cmmode_cmd_func,
	  set_radio_cmmode_cmd,
	  "11n cwmode (ht20|ht20/40|ht40)",
	  "set channel width mode \n"
	  "Radio channel width value 0:ht20 1:ht20/40 2:ht40 "
	 )


{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned short cwmode = 0;
	//fengwenchao add 20110322
	unsigned int max_channel = 0;
	unsigned int min_channel = 0;
	unsigned char current_channel = 0;	
	//fengwenchao add end

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 	

	if(!strcmp(argv[0],"ht20")){
		cwmode = 0;
	}
	else if(!strcmp(argv[0],"ht20/40")){
		cwmode = 1;
	}
	else if(!strcmp(argv[0],"ht40")){
		cwmode = 2;
	}
	else{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}
	
	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

    RadioList_Head = set_radio_cmmode_cmd_11n_cwmode(localid,index,dcli_dbus_connection,type,id,cwmode,&count,&ret,&max_channel,&min_channel,&current_channel);	
	//printf("max_channel  = %d ,min_channel  =  %d ,current_channel = %d ,ret  =  %d,  type = %d\n",max_channel,min_channel,current_channel,ret,type);	
	if(type==0)
		{
			if(ret == 0)
				vty_out(vty,"Radio %d-%d set cwmode %s successfully .\n",id/L_RADIO_NUM,id%L_RADIO_NUM,argv[0]);
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio id does not exist\n");
			else if(ret == RADIO_IS_DISABLE)
				vty_out(vty,"<error> radio is disable, please enable it first\n");
			else if(ret == WTP_IS_NOT_BINDING_WLAN_ID)
				vty_out(vty,"<error> radio is not binging wlan\n");
			else if(ret == RADIO_CHANNEL_OFFSET_NEED_BE_RESET)
				vty_out(vty,"<error> channel offset should be set none\n");
			//fengwenchao add 20110323
			else if(ret == CHANNEL_CROSS_THE_BORDER)
			{				
				vty_out(vty,"<error>the current radio channel is %d which is cross the border the max channel %d ,you are not allowed to set channel offset up!!\n",current_channel,max_channel);
			
				/*else if(current_channel < min_channel)
				{
					vty_out(vty,"<error>the current radio channel is %d which is cross the border the min channel %d ,you are not allowed to set channel offset down!!\n",current_channel,min_channel);
				}*/
			}			
			//fengwenchao add end			
			else
				vty_out(vty,"<error>  %d\n",ret);		
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set cwmode %s successfully .\n",id,argv[0]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}	

	return CMD_SUCCESS;

}

#else
DEFUN(set_radio_cmmode_cmd_func,
	  set_radio_cmmode_cmd,
	  "11n cwmode (ht20|ht20/40|ht40)",
	  "set channel width mode \n"
	  "Radio channel width value 0:ht20 1:ht20/40 2:ht40 "
	 )


{
	unsigned int radio_id; 
	unsigned int mode;
	unsigned short cwmode;
	int ret = 0;
	//fengwenchao add 20110322
	unsigned int max_channel = 0;
	unsigned int min_channel = 0;
	unsigned char current_channel = 0;	
	//fengwenchao add end
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;	
	
    /*rtsthre= atoi(argv[0]);*/
	//ret = parse_int_ID((char *)argv[0],&mode);
	if(!strcmp(argv[0],"ht20")){
		cwmode = 0;
	}
	else if(!strcmp(argv[0],"ht20/40")){
		cwmode = 1;
	}
	else if(!strcmp(argv[0],"ht40")){
		cwmode = 2;
	}
	else{}
	
	//cwmode = (unsigned short)mode;
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}


	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_CMMODE);
		
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_UINT16,&cwmode,
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
		//fengwenchao add 20110324
		if(ret == CHANNEL_CROSS_THE_BORDER){
		 dbus_message_iter_next(&iter);	
		 dbus_message_iter_get_basic(&iter,&max_channel);
		 dbus_message_iter_next(&iter);	    
		 dbus_message_iter_get_basic(&iter,&min_channel);
		 dbus_message_iter_next(&iter);	    
		 dbus_message_iter_get_basic(&iter,&current_channel);		 
		 }
		//fengwenchao add end
		if(ret == 0)
			vty_out(vty,"Radio %d-%d set cwmode %s successfully .\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0]);
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"<error> radio id does not exist\n");
		else if(ret == RADIO_IS_DISABLE)
			vty_out(vty,"<error> radio is disable, please enable it first\n");
		else if(ret == WTP_IS_NOT_BINDING_WLAN_ID)
			vty_out(vty,"<error> radio is not binging wlan\n");
		else if(ret == RADIO_MODE_IS_11N)
			vty_out(vty,"<error> radio mode is not 11N ,don't support this command\n");
		//fengwenchao add 20110323
		else if(ret == CHANNEL_CROSS_THE_BORDER)
		{
			vty_out(vty,"<error>the current radio channel is %d which is cross the border the max channel %d ,you are not allowed to set channel offset up!!  Please turn down channel\n",current_channel,max_channel);

			/*else if(min_channel)
			{
				vty_out(vty,"<error>the current radio channel is %d which is cross the border the min channel %d ,you are not allowed to set channel offset down!!\n",current_channel,min_channel);
			}*/
		}			
		//fengwenchao add end
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;

}
#endif
#if _GROUP_POLICY
DEFUN(set_radio_sector_cmd_func,
	  set_radio_sector_cmd,
	  "set radio sector LIST (enable|disable)",
	  "set sector [value]\n"
	  "Radio sector width value\n"
	 )
{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned int policy = 0;
	unsigned short hex_id = 0;

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 	
	
	ret = parse_radio_sector_list((char*)argv[0],&hex_id);

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
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		vty_out(vty,"<error> input parameter only should be 0,1,2,3 one or more of them,such as 1,2 or 1,2,3.\n");
		return CMD_SUCCESS;
	}

	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = set_radio_netgear_set_radio_sector_power_set_radio_sector_list(localid,index,dcli_dbus_connection,type,id,hex_id,policy,
		WID_DBUS_RADIO_SECTOR_SET_CMD,&count,&ret);	

	if(type==0)
		{
			if(ret == -1)
				{
					cli_syslog_info("<error> failed get reply.\n");
				}
			else if(ret == 0)
					vty_out(vty,"set radio sector %s successfully .\n",argv[0]);
			else if(ret == RADIO_ID_NOT_EXIST)
					vty_out(vty,"<error> radio id does not exist\n");
			else if(ret == RADIO_NOT_SUPPORT_COMMAND)
					vty_out(vty,"<error> radio not support this command\n");
			else
				vty_out(vty,"<error>  %d\n",ret);
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set radio sector %s successfully .\n",id,argv[0]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}
	return CMD_SUCCESS;

}

#else
DEFUN(set_radio_sector_cmd_func,
	  set_radio_sector_cmd,
	  "set radio sector LIST (enable|disable)",
	  "set sector [value]\n"
	  "Radio sector width value\n"
	 )
{
	unsigned int radio_id; 
	unsigned int wlanid = 0; 
	int policy = 0;
	int ret = 0;
	unsigned short hex_id = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;	
	
    /*rtsthre= atoi(argv[0]);*/
	//ret = radio_sector_parse_func((char *)argv[0],&sectorvalue);
	ret = parse_radio_sector_list((char*)argv[0],&hex_id);
	//vty_out(vty,"hex_id is %u\n",hex_id);
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
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		vty_out(vty,"<error> input parameter only should be 0,1,2,3 one or more of them,such as 1,2 or 1,2,3.\n");
		return CMD_SUCCESS;
	}
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ret = radio_set_method_function(localid,index,radio_id,wlanid,hex_id,policy,NULL,WID_DBUS_RADIO_SECTOR_SET_CMD,dcli_dbus_connection);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0)
		vty_out(vty,"set radio sector %s successfully .\n",argv[0]);
	else if(ret == RADIO_ID_NOT_EXIST)
		vty_out(vty,"<error> radio id does not exist\n");
	else if(ret == RADIO_NOT_SUPPORT_COMMAND)
		vty_out(vty,"<error> radio not support this command\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
//	dbus_message_unref(reply);
	return CMD_SUCCESS;

}
#endif
#if 0
DEFUN(set_radio_sector_cmd_func,
	  set_radio_sector_cmd,
	  "set radio sector LIST (enable|disable)",
	  "set sector [value]\n"
	  "Radio sector width value\n"
	 )
{
	unsigned int radio_id; 
	unsigned int wlanid = 0; 
	unsigned short sectorvalue;
	int ret = 0;
	int num = 0;
	int hex_id = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;	
	
    /*rtsthre= atoi(argv[0]);*/
	//ret = radio_sector_parse_func((char *)argv[0],&sectorvalue);
	int *sectorID[SECTOR_NUM];
	ret = parse_radio_sector_list((char*)argv[0],sectorID,&num);
	int i = 0;
	int policy = 0;
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
	
	if(ret == 0)
	{	
		conversion_parameter_hex(sectorID,num,policy,&hex_id);

	}
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		vty_out(vty,"<error> input parameter only should be 0,1,2,3 one or more of them,such as 1,2 or 1,2,3.\n");
		return CMD_SUCCESS;
	}
	int index = 0;
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		radio_id = (int)vty->index_sub;
	}
	ret = radio_set_method_function(localid,index,radio_id,wlanid,hex_id,policy,NULL,WID_DBUS_RADIO_SECTOR_SET_CMD,dcli_dbus_connection);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0)
		vty_out(vty,"set radio sector %s successfully .\n",argv[0]);
	else if(ret == RADIO_ID_NOT_EXIST)
		vty_out(vty,"<error> radio id does not exist\n");
	else if(ret == RADIO_NOT_SUPPORT_COMMAND)
		vty_out(vty,"<error> radio not support this command\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
//	dbus_message_unref(reply);
	return CMD_SUCCESS;

}
#endif

#if _GROUP_POLICY
DEFUN(set_radio_sector_power_cmd_func,
	  set_radio_sector_power_cmd,
	  "set radio sectorid (0|1|2|3|all) power VALUE",
	  "set sector power [value]\n"
	  "Radio sector power value\n"
	 )
{

	unsigned short sectorid = 0;
	unsigned int sectorvalue = 0;
	
	int i = 0;
	int ret = 0;
	int ret2= 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 

	ret = radio_sector_parse_func((char *)argv[0],&sectorid);
	ret2 = parse_int((char *)argv[1],&sectorvalue);
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		vty_out(vty,"<error> input parameter only should be 0,1,2,3,all.\n");
		return CMD_SUCCESS;
	}
	if (ret2 != WID_DBUS_SUCCESS){	
		vty_out(vty,"<error> input parameter %s error\n",argv[1]);
		vty_out(vty,"<error> input parameter only should be 0-30.\n");
		return CMD_SUCCESS;
	}

	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	RadioList_Head = set_radio_netgear_set_radio_sector_power_set_radio_sector_list(localid,index,dcli_dbus_connection,type,id,sectorid,sectorvalue,
		WID_DBUS_RADIO_SECTOR_POWER_SET_CMD,&count,&ret);	

	if(type==0)
		{
			if(ret == -1)
				{
					cli_syslog_info("<error> failed get reply.\n");
				}
			else if(ret == 0)
					vty_out(vty,"set radio sector %s power %d successfully .\n",argv[0],sectorvalue);
			else if(ret == RADIO_ID_NOT_EXIST)
					vty_out(vty,"<error> radio id does not exist\n");
			else if(ret == WTP_ID_NOT_EXIST)
					vty_out(vty,"<error> wtp id does not exist\n");
			else if(ret == RADIO_NOT_SUPPORT_COMMAND)
					vty_out(vty,"<error> radio not support this command\n");
			else if((ret == RADIO_SECTOR_DISABLE)&&(sectorid ==4))
					vty_out(vty,"<error> some of sectors is disable,enable it/them first.\n");
			else if(ret == RADIO_SECTOR_DISABLE)
					vty_out(vty,"<error> radio sector %d is disable,enable it first.\n",sectorid);
			else
					vty_out(vty,"<error>  %d\n",ret);
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set radio sector %s power %d successfully .\n",id,argv[0],sectorvalue);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}	

	return CMD_SUCCESS;

}

#else
DEFUN(set_radio_sector_power_cmd_func,
	  set_radio_sector_power_cmd,
	  "set radio sectorid (0|1|2|3|all) power VALUE",
	  "set sector power [value]\n"
	  "Radio sector power value\n"
	 )
{
	unsigned int radio_id; 
	unsigned int wlanid = 0; 
	unsigned short sectorid = 0;
	unsigned int sectorvalue;
	int ret,ret2;
	ret=ret2=0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;	
	
    /*rtsthre= atoi(argv[0]);*/
	ret = radio_sector_parse_func((char *)argv[0],&sectorid);
	ret2 = parse_int((char *)argv[1],&sectorvalue);
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		vty_out(vty,"<error> input parameter only should be 0,1,2,3,all.\n");
		return CMD_SUCCESS;
	}
	if (ret2 != WID_DBUS_SUCCESS){	
		vty_out(vty,"<error> input parameter %s error\n",argv[1]);
		vty_out(vty,"<error> input parameter only should be 0-30.\n");
		return CMD_SUCCESS;
	}
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ret = radio_set_method_function(localid,index,radio_id,wlanid,sectorid,sectorvalue,NULL,WID_DBUS_RADIO_SECTOR_POWER_SET_CMD,dcli_dbus_connection);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0)
		vty_out(vty,"set radio sector %s power %d successfully .\n",argv[0],sectorvalue);
	else if(ret == RADIO_ID_NOT_EXIST)
		vty_out(vty,"<error> radio id does not exist\n");
	else if(ret == WTP_ID_NOT_EXIST)
		vty_out(vty,"<error> wtp id does not exist\n");
	else if(ret == RADIO_NOT_SUPPORT_COMMAND)
		vty_out(vty,"<error> radio not support this command\n");
	else if((ret == RADIO_SECTOR_DISABLE)&&(sectorid ==4))
		vty_out(vty,"<error> some of sectors is disable,enable it/them first.\n");
	else if(ret == RADIO_SECTOR_DISABLE)
		vty_out(vty,"<error> radio sector %d is disable,enable it first.\n",sectorid);
	else
		vty_out(vty,"<error>  %d\n",ret);
//	dbus_message_unref(reply);
	return CMD_SUCCESS;

}
#endif
#if _GROUP_POLICY
DEFUN(set_radio_netgear_supper_g_cmd_func,
	  set_radio_netgear_supper_g_cmd,
	  "set (bursting|fastFrame|compression) (enable|disable)",
	  "set (bursting|fastFrame|compression) [value]\n"
	  "set bursting enable\n"
	 )
{
	unsigned short super_g_type = 0;
	unsigned int super_g_state = 0;
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 

	
	ret = radio_Netgear_super_g_technology_parse_func((char *)argv[0],&super_g_type);
	if (!strcmp(argv[1],"enable"))
	{
		super_g_state = 1; 
	}
	else if (!strcmp(argv[1],"disable"))
	{
		super_g_state = 0; 
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		vty_out(vty,"<error> input parameter only should be (bursting|fastFrame|compression).\n");
		return CMD_SUCCESS;
	}

	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (int)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = set_radio_netgear_set_radio_sector_power_set_radio_sector_list(localid,index,dcli_dbus_connection,type,id,super_g_type,super_g_state,
		WID_DBUS_RADIO_NETGEAR_G_SET_CMD,&count,&ret);
	

	if(type==0)
		{
			if(ret == -1)
				{
					cli_syslog_info("<error> failed get reply.\n");
				}
			else if(ret == 0)
					vty_out(vty,"set %s %s successfully .\n",argv[0],argv[1]);
			else if(ret == RADIO_ID_NOT_EXIST)
					vty_out(vty,"<error> radio id does not exist\n");
			else if(ret == WTP_ID_NOT_EXIST)
					vty_out(vty,"<error> wtp id does not exist\n");
			else if(ret == RADIO_NOT_SUPPORT_COMMAND)
					vty_out(vty,"<error> radio not support this command\n");
			else
					vty_out(vty,"<error>  %d\n",ret);
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set %s %s successfully .\n",id,argv[0],argv[1]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}	
	return CMD_SUCCESS;

}

#else
DEFUN(set_radio_netgear_supper_g_cmd_func,
	  set_radio_netgear_supper_g_cmd,
	  "set (bursting|fastFrame|compression) (enable|disable)",
	  "set (bursting|fastFrame|compression) [value]\n"
	  "set bursting enable\n"
	 )
{
	unsigned int radio_id; 
	unsigned int wlanid = 0; 
	//unsigned short sectorid;
	//unsigned short sectorvalue;
	unsigned short super_g_type;
	unsigned int super_g_state;
	int ret,ret2;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;	
	
    /*rtsthre= atoi(argv[0]);*/
	ret = radio_Netgear_super_g_technology_parse_func((char *)argv[0],&super_g_type);
	if (!strcmp(argv[1],"enable"))
	{
		super_g_state = 1; 
	}
	else if (!strcmp(argv[1],"disable"))
	{
		super_g_state = 0; 
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		vty_out(vty,"<error> input parameter only should be (bursting|fastFrame|compression).\n");
		return CMD_SUCCESS;
	}

	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ret = radio_set_method_function(localid,index,radio_id,wlanid,super_g_type,super_g_state,NULL,WID_DBUS_RADIO_NETGEAR_G_SET_CMD,dcli_dbus_connection);

	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0)
		vty_out(vty,"set %s %s successfully .\n",argv[0],argv[1]);
	else if(ret == RADIO_ID_NOT_EXIST)
		vty_out(vty,"<error> radio id does not exist\n");
	else if(ret == WTP_ID_NOT_EXIST)
		vty_out(vty,"<error> wtp id does not exist\n");
	else if(ret == RADIO_NOT_SUPPORT_COMMAND)
		vty_out(vty,"<error> radio not support this command\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
//	dbus_message_unref(reply);
	return CMD_SUCCESS;

}
#endif
#if _GROUP_POLICY
DEFUN(set_wds_bridge_distance_cmd_func,
	  set_wds_bridge_distance_cmd,
	  "set wds bridge distance DISTANCE",          //fengwenchao change <0-31> to DISTANCE  20110504
	  "set wds bridge distance \n"
	  "distance of wds bridges\n"
	  "distance of wds bridges\n"
	  "distance of wds bridges\n"
	  "distance <0-31>\n"                                    //fengwenchao add 20110504	  
	 )
{
	int distance = 0;
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 	
	
	ret = parse_int_ID((char *)argv[0],&distance);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}


	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		
	RadioList_Head = set_wds_bridge_distance_cmd_set_distance(localid,index,dcli_dbus_connection,type,id,distance,&count,&ret);

	if(type==0)
		{
			if(ret == 0)
				vty_out(vty,"set wds bridge distance %s successfully .\n",argv[0]);
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio id does not exist\n");
			else if (ret == RADIO_NOT_SUPPORT_COMMAND)
				vty_out(vty, "<warning> this radio not supports those commands\n");
			else
				vty_out(vty,"<error>  %d\n",ret);
		}

	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set wds bridge distance %s successfully .\n",id,argv[0]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}	

	return CMD_SUCCESS;
}

#else
DEFUN(set_wds_bridge_distance_cmd_func,
	  set_wds_bridge_distance_cmd,
	  "set wds bridge distance DISTANCE",       //fengwenchao change <0-31> to DISTANCE  20110504
	  "set wds bridge distance \n"
	  "distance of wds bridges\n"
	  "distance of wds bridges\n"
	  "distance of wds bridges\n"
	  "distance <0-31>\n"                                    //fengwenchao add 20110504
	 )
{
	unsigned int radio_id; 
	int ret;
	int distance;
	
	ret = parse_int_ID((char *)argv[0],&distance);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}


	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ret = radio_set_wds_bridge_distance(localid,index,radio_id,distance,dcli_dbus_connection);
	if(ret == 0)
		vty_out(vty,"set wds bridge distance %s successfully .\n",argv[0]);
	else if(ret == RADIO_ID_NOT_EXIST)
		vty_out(vty,"<error> radio id does not exist\n");
	else if (ret == RADIO_NOT_SUPPORT_COMMAND)
		vty_out(vty, "<warning> this radio not supports those commands\n");
	else
		vty_out(vty,"<error>  %d\n",ret);

	return CMD_SUCCESS;
}
#endif
#if _GROUP_POLICY
DEFUN(wds_remote_brmac_cmd_func,
	  wds_remote_brmac_cmd,
	  "(add|del) wds remote brmac MAC ",
	  "add or del wds remote brmac\n"
	  "eg: add wds remote brmac aa:bb:cc:dd:ee:ff\n"
	 )
{
	int op_ret = 0;
	int is_add = 0;
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned char macAddr[MAC_LEN];

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 	
	
	int len = strlen(argv[0]);
	if(len <= 3 && strncmp(argv[0],"add",len) == 0){
		is_add = 1;
	}else if(len <= 3 && strncmp(argv[0],"del",len) == 0){
		is_add = 0;
	}else{
    	vty_out(vty,"Unkown Command!\n");
		return CMD_WARNING;		
	}
	
	op_ret = wid_parse_mac_addr(argv[1],&macAddr);

	if (NPD_FAIL == op_ret) {
    	vty_out(vty,"% Bad Parameter,Unknow mac addr format!\n");
		return CMD_WARNING;
	}

	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type= 1;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	RadioList_Head = wds_remote_brmac_cmd_add_del_MAC(localid,index,dcli_dbus_connection,type,id,is_add,macAddr,
		&count,&ret);

	if(type==0)
		{
			if(ret == 0)
				vty_out(vty,"%s wds remote brmac %s successfully .\n",argv[0],argv[1]);
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio id does not exist\n");
			else if (ret == RADIO_NOT_SUPPORT_COMMAND)
				vty_out(vty, "<warning> this radio not supports those commands\n");
			else
				vty_out(vty,"<error>  %d\n",ret);
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d %s wds remote brmac %s successfully .\n",id,argv[0],argv[1]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}	


	return CMD_SUCCESS;
}

#else
DEFUN(wds_remote_brmac_cmd_func,
	  wds_remote_brmac_cmd,
	  "(add|del) wds remote brmac MAC ",
	  "add or del wds remote brmac\n"
	  "eg: add wds remote brmac aa:bb:cc:dd:ee:ff\n"
	 )
{
	unsigned int radio_id = 0; 
	int ret;
	int op_ret;
	unsigned char macAddr[MAC_LEN];
	int is_add = 0;
	int len = strlen(argv[0]);
	if(len <= 3 && strncmp(argv[0],"add",len) == 0){
		is_add = 1;
	}else if(len <= 3 && strncmp(argv[0],"del",len) == 0){
		is_add = 0;
	}else{
    	vty_out(vty,"Unkown Command!\n");
		return CMD_WARNING;		
	}
	
	op_ret = wid_parse_mac_addr(argv[1],&macAddr);

	if (NPD_FAIL == op_ret) {
    	vty_out(vty,"% Bad Parameter,Unknow mac addr format!\n");
		return CMD_WARNING;
	}

	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ret = radio_set_wds_remote_brmac(localid,index,radio_id,is_add,macAddr,dcli_dbus_connection);
	if(ret == 0)
		vty_out(vty,"%s wds remote brmac %s successfully .\n",argv[0],argv[1]);
	else if(ret == RADIO_ID_NOT_EXIST)
		vty_out(vty,"<error> radio id does not exist\n");
	else if (ret == RADIO_NOT_SUPPORT_COMMAND)
		vty_out(vty, "<warning> this radio not supports those commands\n");
	else
		vty_out(vty,"<error>  %d\n",ret);

	return CMD_SUCCESS;
}
#endif
#if _GROUP_POLICY
DEFUN(wds_encryption_type_cmd_func,
	  wds_encryption_type_cmd,
	  "set wds encrption type (disable|wep|aes)",
	  "set wds encrption type\n"
	  "eg: set wds encrption type wep\n"
	 )
{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned int TYPE = 0;

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 	

	if(memcmp(argv[0],"disable",strlen(argv[0])) == 0){
		type = 0;
	}
	else if(memcmp(argv[0],"wep",strlen(argv[0])) == 0){
		type = 1;
	}
	else if(memcmp(argv[0],"aes",strlen(argv[0])) == 0){
		type = 2;
	}else{
		vty_out(vty,"UNKNOW COMMAND\n");
		return CMD_SUCCESS;
	}

	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		TYPE = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		TYPE= 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		TYPE= 1;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	RadioList_Head = wds_encryption_type_cmd_set_wds_encrption_type(localid,index,dcli_dbus_connection,TYPE,id,type,&count,&ret);

	if(TYPE==0)
		{
			if(ret == 0)
				vty_out(vty,"set wds encrption type %s successfully .\n",argv[0]);
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio id does not exist\n");
			else if (ret == RADIO_NOT_SUPPORT_COMMAND)
				vty_out(vty, "<warning> this radio not supports those commands\n");
			else
				vty_out(vty,"<error>  %d\n",ret);
		}
	else if(TYPE==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set wds encrption type %s successfully .\n",id,argv[0]);
					if((count != 0)&&(TYPE == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}	

	return CMD_SUCCESS;
}

#else
DEFUN(wds_encryption_type_cmd_func,
	  wds_encryption_type_cmd,
	  "set wds encrption type (disable|wep|aes)",
	  "set wds encrption type\n"
	  "eg: set wds encrption type wep\n"
	 )
{
	unsigned int radio_id; 
	int ret;
	int op_ret;
	int type = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	if(memcmp(argv[0],"disable",strlen(argv[0])) == 0){
		type = 0;
	}
	else if(memcmp(argv[0],"wep",strlen(argv[0])) == 0){
		type = 1;
	}
	else if(memcmp(argv[0],"aes",strlen(argv[0])) == 0){
		type = 2;
	}else{
		vty_out(vty,"UNKNOW COMMAND\n");
		return CMD_SUCCESS;
	}

	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ret = radio_set_wds_encryption_type(localid,index,radio_id,type,dcli_dbus_connection);
	if(ret == 0)
		vty_out(vty,"set wds encrption type %s successfully .\n",argv[0]);
	else if(ret == RADIO_ID_NOT_EXIST)
		vty_out(vty,"<error> radio id does not exist\n");
	else if (ret == RADIO_NOT_SUPPORT_COMMAND)
		vty_out(vty, "<warning> this radio not supports those commands\n");
	else
		vty_out(vty,"<error>  %d\n",ret);

	return CMD_SUCCESS;
}
#endif
#if _GROUP_POLICY

/*nl add 20100119*/
DEFUN(set_radio_inter_vap_forwarding_cmd_func,
	  set_radio_inter_vap_forwarding_cmd,
	  "set inter-VAP-forwarding (enable|disable)",
	  "set inter VAP forwarding\n"
	  "Radio inter-VAP-forwarding value enable|disable\n"
	 )
{ 
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned char policy =0;   

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 		
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
	
	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type = 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type = 1;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = set_radio_inter_vap_forwarding_cmd_set_inter_VAP_forwarding(localid,index,dcli_dbus_connection,type,id,policy,
		&count,&ret);

	if(type==0)
		{
			if(ret == -1)
				{
					cli_syslog_info("<error> failed get reply.\n");
				}
			else if(ret == WTP_ID_NOT_EXIST)
					vty_out(vty,"<error> wtp isn't existed\n");
			else if(ret == RADIO_ID_NOT_EXIST)
					vty_out(vty,"<error> radio isn't existed\n");
			else if(ret==RADIO_NO_BINDING_WLAN)
					vty_out(vty,"<error> radio doesn't bind wlan \n");
			else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
				vty_out(vty,"<error> radio doesn't bind wlan\n");
			else if(ret == WID_DBUS_SUCCESS)
					vty_out(vty,"set inter_vap_forwarding   successfully!\n");
			else if(ret == RADIO_NOT_SUPPORT_COMMAND)
					vty_out(vty,"<error> radio not support this command\n");
			else
					vty_out(vty,"<error> other error.err id is %d.\n",ret);
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set inter_vap_forwarding   successfully!\n",id);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}
	return CMD_SUCCESS; 
}

#else

/*nl add 20100119*/
DEFUN(set_radio_inter_vap_forwarding_cmd_func,
	  set_radio_inter_vap_forwarding_cmd,
	  "set inter-VAP-forwarding (enable|disable)",
	  "set inter VAP forwarding\n"
	  "Radio inter-VAP-forwarding value enable|disable\n"
	 )
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret = 0;
	unsigned int RadioID;
	unsigned char policy =0;   
		
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
	int localid = 1;
    int slot_id = HostSlotId;
//	char BUSNAME[PATH_LEN];
//	char OBJPATH[PATH_LEN];
//	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		RadioID = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		RadioID = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		RadioID = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#if 0	
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
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
#endif

	ret = radio_set_radio_inter_vap_type(localid,index,RadioID,policy,dcli_dbus_connection);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == WTP_ID_NOT_EXIST)
		vty_out(vty,"<error> wtp isn't existed\n");
	else if(ret == RADIO_ID_NOT_EXIST)
		vty_out(vty,"<error> radio isn't existed\n");
	else if(ret==RADIO_NO_BINDING_WLAN)
		vty_out(vty,"<error> radio doesn't bind wlan \n");
	else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
		vty_out(vty,"<error> radio doesn't bind wlan\n");
	else if(ret == WID_DBUS_SUCCESS)
		vty_out(vty,"set inter_vap_forwarding   successfully!\n");
	else if(ret == RADIO_NOT_SUPPORT_COMMAND)
		vty_out(vty,"<error> radio not support this command\n");
	else
		vty_out(vty,"<error> other error.err id is %d.\n",ret);
//	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}
#endif
#if _GROUP_POLICY
/*nl add 20100121*/
DEFUN(set_radio_intra_vap_forwarding_cmd_func,
	  set_radio_intra_vap_forwarding_cmd,
	  "set intra-VAP-forwarding (enable|disable)",
	  "set intra VAP forwarding\n"
	  "Radio intra-VAP-forwarding value enable|disable\n"
	 )
{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned char policy =0;   

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 
		
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
	
	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index;
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type = 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type = 1;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = set_radio_intra_vap_forwarding_cmd_set_intra_VAP_forwarding(localid,index,dcli_dbus_connection,type,id,policy,
		&count,&ret);


	if(type==0)
		{
			if(ret == WTP_ID_NOT_EXIST)
				vty_out(vty,"<error> wtp isn't existed\n");
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio isn't existed\n");
			else if(ret==RADIO_NO_BINDING_WLAN)
				vty_out(vty,"<error> radio doesn't bind wlan \n");
			else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
				vty_out(vty,"<error> radio doesn't bind wlan\n");
			else if(ret == WID_DBUS_SUCCESS)
				vty_out(vty,"set intra_vap_forwarding   successfully!\n");
			else
				vty_out(vty,"<error> other error\n");			
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set intra_vap_forwarding   successfully!\n",id);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}


	return CMD_SUCCESS; 
}

#else
/*nl add 20100121*/
DEFUN(set_radio_intra_vap_forwarding_cmd_func,
	  set_radio_intra_vap_forwarding_cmd,
	  "set intra-VAP-forwarding (enable|disable)",
	  "set intra VAP forwarding\n"
	  "Radio intra-VAP-forwarding value enable|disable\n"
	 )
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
	unsigned int RadioID;
	unsigned char policy =0;   
		
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
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		RadioID = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		RadioID = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		RadioID = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_INTRA_VAP_FORVARDING_ABLE);


	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&RadioID,
							DBUS_TYPE_BYTE,&policy,
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
		
	if(ret == WTP_ID_NOT_EXIST)
		vty_out(vty,"<error> wtp isn't existed\n");
	else if(ret == RADIO_ID_NOT_EXIST)
		vty_out(vty,"<error> radio isn't existed\n");
	else if(ret==RADIO_NO_BINDING_WLAN)
		vty_out(vty,"<error> radio doesn't bind wlan \n");
	else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
		vty_out(vty,"<error> radio doesn't bind wlan\n");
	else if(ret == WID_DBUS_SUCCESS)
		vty_out(vty,"set intra_vap_forwarding   successfully!\n");
	else
		vty_out(vty,"<error> other error\n");
	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}
#endif
#if _GROUP_POLICY
/*nl add 20100128*/
DEFUN(set_radio_keep_alive_period_cmd_func,
	  set_radio_keep_alive_period_cmd,
	  "set radio keep_alive_period VALUE",
	  "set radio keep_alive_period\n"
	  "Radio keep_alive_period value \n"
	 )
{  
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned int period =0; 

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 

	period = (unsigned int)atoi(argv[0]);
		
	if ((period < 1)||(period > 3600)) 
	{
		vty_out(vty,"<error> input parameter should be 1-3600\n");
		return CMD_SUCCESS;
	}
	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type = 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type = 1;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = set_radio_keep_alive_period_cmd_set_radio_keep_alive_period(localid,index,dcli_dbus_connection,
	type,id,period,&count,&ret);

	if(type==0)
		{
			if(ret == WTP_ID_NOT_EXIST)
				vty_out(vty,"<error> wtp isn't existed\n");
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio isn't existed\n");
			else if(ret==RADIO_NO_BINDING_WLAN)
				vty_out(vty,"<error> radio doesn't bind wlan \n");
			else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
				vty_out(vty,"<error> radio doesn't bind wlan\n");
			else if(ret == WID_DBUS_SUCCESS)
				vty_out(vty,"set keep_alive_period   successfully!\n");
			else if (ret == RADIO_NOT_SUPPORT_COMMAND)
				vty_out(vty, "<warning> this radio not supports those commands\n");
			else
				vty_out(vty,"<error> other error\n");
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set keep_alive_period   successfully!\n",id);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}		

	return CMD_SUCCESS; 
}

#else
/*nl add 20100128*/
DEFUN(set_radio_keep_alive_period_cmd_func,
	  set_radio_keep_alive_period_cmd,
	  "set radio keep_alive_period VALUE",
	  "set radio keep_alive_period\n"
	  "Radio keep_alive_period value \n"
	 )
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
	unsigned int RadioID;
	unsigned int period =0;   

	period = (unsigned int)atoi(argv[0]);
		
	if ((period < 1)||(period > 3600)) 
	{
		vty_out(vty,"<error> input parameter should be 1-3600\n");
		return CMD_SUCCESS;
	}
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		RadioID = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		RadioID = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		RadioID = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_KEEP_ALIVE_PERIOD);


	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&RadioID,
							DBUS_TYPE_UINT32,&period,
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
		
	if(ret == WTP_ID_NOT_EXIST)
		vty_out(vty,"<error> wtp isn't existed\n");
	else if(ret == RADIO_ID_NOT_EXIST)
		vty_out(vty,"<error> radio isn't existed\n");
	else if(ret==RADIO_NO_BINDING_WLAN)
		vty_out(vty,"<error> radio doesn't bind wlan \n");
	else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
		vty_out(vty,"<error> radio doesn't bind wlan\n");
	else if(ret == WID_DBUS_SUCCESS)
		vty_out(vty,"set keep_alive_period   successfully!\n");
	else if (ret == RADIO_NOT_SUPPORT_COMMAND)
		vty_out(vty, "<warning> this radio not supports those commands\n");
	else
		vty_out(vty,"<error> other error\n");
	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}
#endif
#if _GROUP_POLICY

/*nl add 20100129*/
DEFUN(set_radio_keep_alive_idle_time_cmd_func,
	  set_radio_keep_alive_idle_time_cmd,
	  "set radio keep_alive_idle_time VALUE",
	  "set radio keep_alive_idle_time\n"
	  "Radio keep_alive_idle_time value \n"
	 )
{  
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned int period =0; 

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 
	period = (unsigned int)atoi(argv[0]);
		
	if ((period < 1)||(period > 3600)) 
	{
		vty_out(vty,"<error> input parameter should be 1-3600\n");
		return CMD_SUCCESS;
	}

	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type = 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type = 1;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	RadioList_Head = set_radio_keep_alive_idle_time_cmd_set_radio_keep_alive_idle_time(localid,index,dcli_dbus_connection,type,id,period,
		&count,&ret);

	if(type==0)
		{
			if(ret == WTP_ID_NOT_EXIST)
				vty_out(vty,"<error> wtp isn't existed\n");
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio isn't existed\n");
			else if(ret==RADIO_NO_BINDING_WLAN)
				vty_out(vty,"<error> radio doesn't bind wlan \n");
			else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
				vty_out(vty,"<error> radio doesn't bind wlan\n");
			else if(ret == WID_DBUS_SUCCESS)
				vty_out(vty,"set keep_alive_period   successfully!\n");
			else if (ret == RADIO_NOT_SUPPORT_COMMAND)
				vty_out(vty, "<warning> this radio not supports those commands\n");
			else
				vty_out(vty,"<error> other error\n");
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set keep_alive_period   successfully!\n",id);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}	

	return CMD_SUCCESS; 
}

#else

/*nl add 20100129*/
DEFUN(set_radio_keep_alive_idle_time_cmd_func,
	  set_radio_keep_alive_idle_time_cmd,
	  "set radio keep_alive_idle_time VALUE",
	  "set radio keep_alive_idle_time\n"
	  "Radio keep_alive_idle_time value \n"
	 )
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
	unsigned int RadioID;
	unsigned int period =0;   

	period = (unsigned int)atoi(argv[0]);
		
	if ((period < 1)||(period > 3600)) 
	{
		vty_out(vty,"<error> input parameter should be 1-3600\n");
		return CMD_SUCCESS;
	}
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		RadioID = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		RadioID = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		RadioID = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_KEEP_ALIVE_IDLE_TIME);


	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&RadioID,
							DBUS_TYPE_UINT32,&period,
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
		
	if(ret == WTP_ID_NOT_EXIST)
		vty_out(vty,"<error> wtp isn't existed\n");
	else if(ret == RADIO_ID_NOT_EXIST)
		vty_out(vty,"<error> radio isn't existed\n");
	else if(ret==RADIO_NO_BINDING_WLAN)
		vty_out(vty,"<error> radio doesn't bind wlan \n");
	else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
		vty_out(vty,"<error> radio doesn't bind wlan\n");
	else if(ret == WID_DBUS_SUCCESS)
		vty_out(vty,"set keep_alive_period   successfully!\n");
	else if (ret == RADIO_NOT_SUPPORT_COMMAND)
		vty_out(vty, "<warning> this radio not supports those commands\n");
	else
		vty_out(vty,"<error> other error\n");
	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}
#endif
#if _GROUP_POLICY
/*nl add 20100130*/
DEFUN(set_radio_congestion_avoidance_cmd_func,
	  set_radio_congestion_avoidance_cmd,
	  "set radio congestion_avoidance (disable|tail-drop|red|fwred|)",
	  "set radio congestion_avoidance\n"
	  "Radio congestion_avoidance value \n"
	 )
{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned int congestion_av_state = 0; 
	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 
	
	if (!strcmp(argv[0],"disable"))
	{
		congestion_av_state = 0; 
	}
	else if (!strcmp(argv[0],"tail-drop"))
	{
		congestion_av_state = 1; 
	}
	else if (!strcmp(argv[0],"red"))
	{
		congestion_av_state = 2; 
	}
	else if (!strcmp(argv[0],"fwred"))
	{
		congestion_av_state = 3; 
	}
	else{
		vty_out(vty,"UNKNOW COMMAND\n");
		return CMD_SUCCESS;
	}

	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type = 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type = 1;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	RadioList_Head = set_radio_congestion_avoidance_cmd_set_radio_congestion_avoidance(localid,index,dcli_dbus_connection,type,id,
		congestion_av_state,&count,&ret);

	if(type==0)
		{
			if(ret == WTP_ID_NOT_EXIST)
				vty_out(vty,"<error> wtp isn't existed\n");
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio isn't existed\n");
			else if(ret==RADIO_NO_BINDING_WLAN)
				vty_out(vty,"<error> radio doesn't bind wlan \n");
			else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
				vty_out(vty,"<error> radio doesn't bind wlan\n");
			else if(ret == WID_DBUS_SUCCESS)
				vty_out(vty,"set congestion_avoidance successfully!\n");
			else if (ret == RADIO_NOT_SUPPORT_COMMAND)
				vty_out(vty, "<warning> this radio not supports those commands\n");
			else
				vty_out(vty,"<error> other error\n");
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set congestion_avoidance successfully!\n",id);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}


	return CMD_SUCCESS; 
}

#else
/*nl add 20100130*/
DEFUN(set_radio_congestion_avoidance_cmd_func,
	  set_radio_congestion_avoidance_cmd,
	  "set radio congestion_avoidance (disable|tail-drop|red|fwred|)",
	  "set radio congestion_avoidance\n"
	  "Radio congestion_avoidance value \n"
	 )
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
	unsigned int RadioID;
	unsigned int congestion_av_state;


	if (!strcmp(argv[0],"disable"))
	{
		congestion_av_state = 0; 
	}
	else if (!strcmp(argv[0],"tail-drop"))
	{
		congestion_av_state = 1; 
	}
	else if (!strcmp(argv[0],"red"))
	{
		congestion_av_state = 2; 
	}
	else if (!strcmp(argv[0],"fwred"))
	{
		congestion_av_state = 3; 
	}
	else{
		vty_out(vty,"UNKNOW COMMAND\n");
		return CMD_SUCCESS;
	}

	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == RADIO_NODE){
		index = 0;			
		RadioID = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		RadioID = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		RadioID = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_CONGESTION_AVOID_STATE);


	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&RadioID,
							DBUS_TYPE_UINT32,&congestion_av_state,
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
		
	if(ret == WTP_ID_NOT_EXIST)
		vty_out(vty,"<error> wtp isn't existed\n");
	else if(ret == RADIO_ID_NOT_EXIST)
		vty_out(vty,"<error> radio isn't existed\n");
	else if(ret==RADIO_NO_BINDING_WLAN)
		vty_out(vty,"<error> radio doesn't bind wlan \n");
	else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
		vty_out(vty,"<error> radio doesn't bind wlan\n");
	else if(ret == WID_DBUS_SUCCESS)
		vty_out(vty,"set congestion_avoidance successfully!\n");
	else if (ret == RADIO_NOT_SUPPORT_COMMAND)
		vty_out(vty, "<warning> this radio not supports those commands\n");
	else
		vty_out(vty,"<error> other error\n");
	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}
#endif
#if _GROUP_POLICY
DEFUN(wds_wep_key_cmd_func,
	  wds_wep_key_cmd,
	  "set wds wep key KEY",
	  "set wds wep key\n"
	  "eg: set wds wep key 12345\n"
	 )
{
	char *key = NULL;
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 

	int len = strlen(argv[0]);
	if ((len != 5)&&(len != 10)&&(len != 13)&&(len != 26)) {
    	vty_out(vty,"% Bad Parameter,Unknow mac addr format!\n");
		return CMD_WARNING;
	}
	key = (char*)malloc(len+1);
	memset(key, 0, len+1);
	memcpy(key,argv[0],len);

	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type = 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type = 1;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	RadioList_Head = wds_wep_key_cmd_set_wds_wep_key(localid,index,dcli_dbus_connection,type,id,key,&count,&ret);
	
	if(type==0)
		{
			if(ret == 0)
				vty_out(vty,"set wds wep key %s successfully .\n",argv[0]);
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio id does not exist\n");
			else if (ret == RADIO_NOT_SUPPORT_COMMAND)
				vty_out(vty, "<warning> this radio not supports those commands\n");
			else
				vty_out(vty,"<error>  %d\n",ret);
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set wds wep key %s successfully .\n",id,argv[0]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}	

	if(key)
		{
			free(key);
			key=NULL;
		}
	return CMD_SUCCESS;
}

#else
DEFUN(wds_wep_key_cmd_func,
	  wds_wep_key_cmd,
	  "set wds wep key KEY",
	  "set wds wep key\n"
	  "eg: set wds wep key 12345\n"
	 )
{
	unsigned int radio_id; 
	int ret;
	int op_ret;
	unsigned char macAddr[MAC_LEN];
	int len = strlen(argv[0]);
	char *key;
	if ((len != 5)&&(len != 10)&&(len != 13)&&(len != 26)) {
    	vty_out(vty,"% Bad Parameter,Unknow mac addr format!\n");
		return CMD_WARNING;
	}
	key = (char*)malloc(len+1);
	memset(key, 0, len+1);
	memcpy(key,argv[0],len);
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ret = radio_set_wds_wep_key(localid,index,radio_id,key,dcli_dbus_connection);
	free(key);
	if(ret == 0)
		vty_out(vty,"set wds wep key %s successfully .\n",argv[0]);
	else if(ret == RADIO_ID_NOT_EXIST)
		vty_out(vty,"<error> radio id does not exist\n");
	else if (ret == RADIO_NOT_SUPPORT_COMMAND)
		vty_out(vty, "<warning> this radio not supports those commands\n");
	else
		vty_out(vty,"<error>  %d\n",ret);

	return CMD_SUCCESS;
}
#endif
#if _GROUP_POLICY
DEFUN(wds_aes_key_cmd_func,
	  wds_aes_key_cmd,
	  "set wds brmac MAC aes key KEY",
	  "set wds aes key base brmac\n"
	  "eg: set wds brmac aa:bb:cc:dd:ee:ff aes key 12345678123456781234567812345678\n"
	 )
{
	int op_ret;
	unsigned char macAddr[MAC_LEN];
	char *key = NULL;
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 
	int len = strlen(argv[1]);
	if(len != 32){
    	vty_out(vty,"key len must be 32!\n");
		return CMD_WARNING;		
	}
	
	op_ret = wid_parse_mac_addr(argv[0],&macAddr);

	if (NPD_FAIL == op_ret) {
    	vty_out(vty,"% Bad Parameter,Unknow mac addr format!\n");
		return CMD_WARNING;
	}
	key = (char *)malloc(len +1);
	memset(key, 0, len+1);
	memcpy(key, argv[1], len);
	
	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type = 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type = 1;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = wds_aes_key_cmd_setwds_brmac(localid,index,dcli_dbus_connection,type,id,key,macAddr,&count,&ret);
	if(type==0)
		{
			if(ret == 0)
				vty_out(vty,"%s wds remote brmac %s successfully .\n",argv[0],argv[1]);
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio id does not exist\n");
			else if (ret == RADIO_NOT_SUPPORT_COMMAND)
				vty_out(vty, "<warning> this radio not supports those commands\n");
			else
				vty_out(vty,"<error>  %d\n",ret);
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d %s wds remote brmac %s successfully .\n",id,argv[0],argv[1]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}
	if(key)
		{
			free(key);
			key=NULL;
		}

	return CMD_SUCCESS;
}

#else
DEFUN(wds_aes_key_cmd_func,
	  wds_aes_key_cmd,
	  "set wds brmac MAC aes key KEY",
	  "set wds aes key base brmac\n"
	  "eg: set wds brmac aa:bb:cc:dd:ee:ff aes key 12345678123456781234567812345678\n"
	 )
{
	unsigned int radio_id; 
	int ret;
	int op_ret;
	unsigned char macAddr[MAC_LEN];
	int is_add = 0;
	int len = strlen(argv[1]);
	char *key;
	if(len != 32){
    	vty_out(vty,"key len must be 32!\n");
		return CMD_WARNING;		
	}
	
	op_ret = wid_parse_mac_addr(argv[0],&macAddr);

	if (NPD_FAIL == op_ret) {
    	vty_out(vty,"% Bad Parameter,Unknow mac addr format!\n");
		return CMD_WARNING;
	}
	key = (char *)malloc(len +1);
	memset(key, 0, len+1);
	memcpy(key, argv[1], len);
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ret = radio_set_wds_aes_key(localid,index,radio_id,key,macAddr,dcli_dbus_connection);
	free(key);
	if(ret == 0)
		vty_out(vty,"%s wds remote brmac %s successfully .\n",argv[0],argv[1]);
	else if(ret == RADIO_ID_NOT_EXIST)
		vty_out(vty,"<error> radio id does not exist\n");
	else if (ret == RADIO_NOT_SUPPORT_COMMAND)
		vty_out(vty, "<warning> this radio not supports those commands\n");
	else
		vty_out(vty,"<error>  %d\n",ret);

	return CMD_SUCCESS;
}
#endif

DEFUN(set_wtp_list_sta_static_arp_enable_func,
		  set_wtp_list_sta_static_arp_enable_cmd,
		  "set wlan WLANID wtp WTPLIST sta_static_arp (enable|disable) base IFNAME",
		  "sta static arp config\n"
		  "fixed format to input wlan\n"
		  "wlan id\n"
		  "fixed format to input wtp\n"
		  "wtp list\n"
		  "fixed format to input sta_static_arp\n"
		  "enable/disable\n"
		  "enable/disable\n"
		  "fixed format to input base\n"
		  "ifname (like: eth0-1)"
	 )
{	
	int ret;
	unsigned int wtp_id = 0;
	unsigned int i = 0;
	update_wtp_list *wtplist;
    int policy = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned char wlanid;
	char *ifname;
	ret = parse_char_ID((char*)argv[0], &wlanid);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
			    vty_out(vty,"<error> unknown id format\n");
			}
			return CMD_SUCCESS;
	}	
	if(wlanid >= WLAN_NUM || wlanid == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	wtplist = (struct tag_wtpid_list*)malloc(sizeof(struct tag_wtpid_list));
	wtplist->wtpidlist = NULL ; 	
	wtplist->count = 0;
	
	if (!strcmp(argv[1],"all"))
	{
		;	
	}else{
		ret = parse_wtpid_list((char*)argv[1],&wtplist);
		if(ret != 0)
		{
			vty_out(vty, "%% set wtp list error,like 1,8,9-20,33\n");
			destroy_input_wtp_list(wtplist);
			return CMD_WARNING;
		}
		else
		{
			delsame(wtplist);		
		}
	}

	
	if (!strcmp(argv[2],"enable"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[2],"disable"))
	{
		policy = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'enable' or 'disable'\n");
		destroy_input_wtp_list(wtplist);
		return CMD_SUCCESS;
	}
	ifname = malloc(strlen(argv[3])+1);
	memset(ifname, 0, strlen(argv[3])+1);
	memcpy(ifname, argv[3], strlen(argv[3]));

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
	
	ret = dcli_wlan_wtp_list_sta_static_arp(localid,index,policy,wlanid,wtplist,ifname,dcli_dbus_connection);
	destroy_input_wtp_list(wtplist);
	free(ifname);
	ifname = NULL;
	if(ret == 0)
		vty_out(vty,"set wlan %s wtp %s sta_static_arp %s base %s successfully\n",argv[0],argv[1],argv[2],argv[3]);
	else if(ret == APPLY_IF_FAIL){
		vty_out(vty,"interface %s no exist\n",argv[3]);
	}/*wcl add for AUTELAN-2836*/
	else if (ret == WLAN_ID_NOT_EXIST)
	{
		vty_out(vty, "<error> wlan %d was not created before\n", wlanid);
	}
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	return CMD_SUCCESS; 

}
#if _GROUP_POLICY
DEFUN(set_wtp_sta_static_arp_enable_func,
		  set_wtp_sta_static_arp_enable_cmd,
	 	  "set wlan WLANID sta_static_arp (enable|disable) base IFNAME",
		  "sta static arp config\n"
		  "wlan id\n"
		  "enable/disable\n"
	 	  "ifname (like: eth0-1)"
	 )
{	
	unsigned char wlanid;
	char *ifname;
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned int policy = 0;
	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 
	ret = parse_char_ID((char*)argv[0], &wlanid);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
			    vty_out(vty,"<error> unknown id format\n");
			}
			return CMD_SUCCESS;
	}	
	if(wlanid >= WLAN_NUM || wlanid == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
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
	ifname = malloc(strlen(argv[2])+1);
	memset(ifname, 0, strlen(argv[2])+1);
	memcpy(ifname, argv[2], strlen(argv[2]));

	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type = 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type = 1;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = set_wtp_sta_static_arp_enable_cmd_set_wlan_sta_static_arp(localid,index,dcli_dbus_connection,type,id,policy,wlanid,ifname,
		&count,&ret);

	if(type==0)
		{
			if(ret == 0)
				vty_out(vty,"set wlan %s sta_static_arp %s base %s successfully\n",argv[0],argv[1],argv[2]);
			else
				vty_out(vty,"<error>  %d\n",ret);
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set wlan %s sta_static_arp %s base %s successfully\n",id,argv[0],argv[1],argv[2]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}
	
	return CMD_SUCCESS; 

}

#else
DEFUN(set_wtp_sta_static_arp_enable_func,
		  set_wtp_sta_static_arp_enable_cmd,
	 	  "set wlan WLANID sta_static_arp (enable|disable) base IFNAME",
		  "sta static arp config\n"
		  "fixed format to input wlan\n"
		  "wlan id\n"
		  "fixed format to input sta_static_arp\n"
		  "enable/disable\n"
		  "enable/disable\n"		  
		  "fixed format to input base\n"
	 	  "ifname (like: eth0-1)"
	 )
{	
	int ret;
	unsigned int radioid = 0;
	unsigned int i = 0;
    int policy = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned char wlanid;
	char *ifname;
	ret = parse_char_ID((char*)argv[0], &wlanid);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
			    vty_out(vty,"<error> unknown id format\n");
			}
			return CMD_SUCCESS;
	}	
	if(wlanid >= WLAN_NUM || wlanid == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
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
	ifname = malloc(strlen(argv[2])+1);
	memset(ifname, 0, strlen(argv[2])+1);
	memcpy(ifname, argv[2], strlen(argv[2]));

	if(vty->node == RADIO_NODE){
		index = 0;			
		radioid = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		radioid = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radioid = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ret = dcli_wlan_wtp_sta_static_arp(localid,index,policy,wlanid,radioid,ifname,dcli_dbus_connection);
	if(ifname){
		free(ifname);
		ifname = NULL;   //fengwenchao add
		}
	if(ret == 0)
		vty_out(vty,"set wlan %s sta_static_arp %s base %s successfully\n",argv[0],argv[1],argv[2]);
	else if(ret == APPLY_IF_FAIL){
		vty_out(vty,"interface %s no exist\n",argv[2]);
	}/*wcl add for AUTELAN-2832*/
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	return CMD_SUCCESS; 

}
#endif

#if 0
DEFUN(set_radio_11n_ampdu_able_func,
	  set_radio_11n_ampdu_able_cmd,
	  "11n ampdu (enable|disable)",
	  "set ampdu switch \n"
	  "11n Radio ampdu able\n"
	 )


{
	unsigned int radio_id; 
	int ret;
	unsigned char policy = 0;
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
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		radio_id = (int)vty->index_sub;
	}
	
	ret = dcli_radio_11n_set_ampdu_able(localid,index,policy,radio_id,dcli_dbus_connection);

	if(ret == 0)
		vty_out(vty,"radio %d-%d set ampdu %s successfully .\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0]);
	else if(ret == RADIO_ID_NOT_EXIST)
		vty_out(vty,"<error> radio id does not exist\n");
	else if(ret == RADIO_NO_BINDING_WLAN)
		vty_out(vty,"<error> radio is not binding wlan, please bind it first\n");
	else if(ret == RADIO_IS_DISABLE)
		vty_out(vty,"<error> radio is disable, please enable it first\n");
	else if(ret == RADIO_MODE_IS_11N)
			vty_out(vty,"<error> radio mode is not 11n,don't support this op\n");
	else
		vty_out(vty,"<error>  %d\n",ret);

	return CMD_SUCCESS;

}

DEFUN(set_radio_11n_ampdu_limit_func,
	  set_radio_11n_ampdu_limit_cmd,
	  "11n ampdu limit VALUE",
	  "set ampdu switch \n"
	  "11n Radio ampdu able\n"
	 )
{
	unsigned int radio_id; 
	unsigned int limit;
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;	
	ret = parse_int_ID((char *)argv[0],&limit);
	if(ret != 0){
		vty_out(vty,"<error> input patameter error\n");
		return CMD_SUCCESS;
	}
	else if((ret == 0)&&((limit<1024)||(limit>65535))){
		vty_out(vty,"<error> input patameter error,limit should be 1024-65535\n");
		return CMD_SUCCESS;
	}

	int index = 0;
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		radio_id = (int)vty->index_sub;
	}
	
	ret = dcli_radio_11n_set_ampdu_limit(localid,index,radio_id,limit,dcli_dbus_connection);

	if(ret == 0)
		vty_out(vty,"radio %d-%d set ampdu %s successfully .\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0]);
	else if(ret == RADIO_ID_NOT_EXIST)
		vty_out(vty,"<error> radio id does not exist\n");
	else if(ret == RADIO_NO_BINDING_WLAN)
		vty_out(vty,"<error> radio is not binding wlan, please bind it first\n");
	else if(ret == RADIO_IS_DISABLE)
		vty_out(vty,"<error> radio is disable, please enable it first\n");
	else if(ret == RADIO_AMPDU_DISABLE){
		vty_out(vty,"<notice> ampdu is disable, we just stored the limit you set,but we don't send it to ap,\n");
		vty_out(vty,"		  until you enable the ampdu!\n");
	}
	else if(ret == RADIO_MODE_IS_11N)
		vty_out(vty,"<error> radio mode is not 11n,don't support this op\n");
	else
		vty_out(vty,"<error>  %d\n",ret);

	return CMD_SUCCESS;

}
#endif
#if _GROUP_POLICY
/* zhangshu add for set a-mpdu | a-msdu,  2010-10-09 */
DEFUN(set_radio_11n_ampdu_able_func,
	  set_radio_11n_ampdu_able_cmd,
	  "11n (ampdu|amsdu) (enable|disable)",
	  "set ampdu|amsdu switch \n"
	  "enable|disable\n"
	 )

{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int TYPE = 0;
	unsigned char type = 0;
	unsigned char policy = 0;
	
	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 
    if (!strcmp(argv[0],"ampdu"))
	{
		type = 1;	
	}
	else if (!strcmp(argv[0],"amsdu"))
	{
		type = 2;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'ampdu' or 'amsdu'\n");
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
	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		TYPE = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		TYPE = 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		TYPE = 1;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	RadioList_Head = set_radio_11n_ampdu_able_cmd_11n(localid,index,dcli_dbus_connection,TYPE,id,policy,type,
		&count,&ret);

	if(TYPE==0)
		{
			if(ret == 0)
				vty_out(vty,"radio %d-%d set %s %s successfully .\n",id/L_RADIO_NUM,id%L_RADIO_NUM,argv[0],argv[1]);
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio id does not exist\n");
			else if(ret == RADIO_NO_BINDING_WLAN)
				vty_out(vty,"<error> radio is not binding wlan, please bind it first\n");
			else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
				vty_out(vty,"<error> radio doesn't bind wlan\n");
			else if(ret == RADIO_IS_DISABLE)
				vty_out(vty,"<error> radio is disable, please enable it first\n");
			else if(ret == RADIO_MODE_IS_11N)
				vty_out(vty,"<error> radio mode is not 11n,don't support this op\n");
    		else if(ret == RADIO_11N_AMSDU_MUTEX)
          	 	vty_out(vty,"<error> amsdu switch is enable, please disable it first\n");
    		else if(ret == RADIO_11N_AMPDU_MUTEX)
           	 	vty_out(vty,"<error> ampdu switch is enable, please disable it first\n");
			else
				vty_out(vty,"<error>  %d\n",ret);
		}
	else if(TYPE==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set %s %s successfully .\n",id,argv[0],argv[1]);
					if((count != 0)&&(TYPE == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}
	return CMD_SUCCESS;
}

#else
/* zhangshu add for set a-mpdu | a-msdu,  2010-10-09 */
DEFUN(set_radio_11n_ampdu_able_func,
	  set_radio_11n_ampdu_able_cmd,
	  "11n (ampdu|amsdu) (enable|disable)",
	  "set ampdu|amsdu switch \n"
	  "enable|disable\n"
	 )


{
	unsigned int radio_id; 
	int ret;
	unsigned char policy = 0;
	unsigned char type = 0;

    if (!strcmp(argv[0],"ampdu"))
	{
		type = 1;	
	}
	else if (!strcmp(argv[0],"amsdu"))
	{
		type = 2;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'ampdu' or 'amsdu'\n");
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

	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ret = dcli_radio_11n_set_ampdu_able(localid,index,policy,radio_id,type,dcli_dbus_connection);

	if(ret == 0)
		vty_out(vty,"radio %d-%d set %s %s successfully .\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0],argv[1]);
	else if(ret == RADIO_ID_NOT_EXIST)
		vty_out(vty,"<error> radio id does not exist\n");
	else if(ret == RADIO_NO_BINDING_WLAN)
		vty_out(vty,"<error> radio is not binding wlan, please bind it first\n");
	else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
		vty_out(vty,"<error> radio doesn't bind wlan\n");
	else if(ret == RADIO_IS_DISABLE)
		vty_out(vty,"<error> radio is disable, please enable it first\n");
	else if(ret == RADIO_MODE_IS_11N)
			vty_out(vty,"<error> radio mode is not 11n,don't support this op\n");
    else if(ret == RADIO_11N_AMSDU_MUTEX)
            vty_out(vty,"<error> amsdu switch is enable, please disable it first\n");
    else if(ret == RADIO_11N_AMPDU_MUTEX)
            vty_out(vty,"<error> ampdu switch is enable, please disable it first\n");
	else
		vty_out(vty,"<error>  %d\n",ret);

	return CMD_SUCCESS;

}
#endif
#if _GROUP_POLICY
DEFUN(set_radio_11n_ampdu_limit_func,
	  set_radio_11n_ampdu_limit_cmd,
	  "11n (ampdu|amsdu) limit VALUE",
	  "set ampdu|amsdu limit \n"
	  "ampdu:1024-65535 amsdu:2290-4096\n"
	 )
{	
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int TYPE = 0;
	unsigned char type = 0;
	unsigned int limit = 0;
	
	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 

    if (!strcmp(argv[0],"ampdu"))
	{
		type = 1;	
	}
	else if (!strcmp(argv[0],"amsdu"))
	{
		type = 2;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'ampdu' or 'amsdu'\n");
		return CMD_SUCCESS;
	}
	
	ret = parse_int_ID((char *)argv[1],&limit);
	if(ret != 0){
		vty_out(vty,"<error> input patameter error\n");
		return CMD_SUCCESS;
	}
	else if(ret == 0)
	{
    	if((type == 1)&&((limit<1024)||(limit>65535)))
    	{
    		vty_out(vty,"<error> input patameter error,ampdu limit should be 1024-65535\n");
    		return CMD_SUCCESS;
    	}
    	else if((type == 2)&&((limit<2290)||(limit>4096)))
    	{
            vty_out(vty,"<error> input patameter error,amsdu limit should be 2290-4096\n");
    		return CMD_SUCCESS;
    	}
	}
	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		TYPE = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		TYPE = 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		TYPE = 1;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	RadioList_Head = set_radio_11n_ampdu_limit_cmd_11n(localid,index,dcli_dbus_connection,TYPE,id,limit,type,
		&count,&ret);

	if(TYPE==0)
		{
			if(ret == 0)
				vty_out(vty,"radio %d-%d set %s %s successfully .\n",id/L_RADIO_NUM,id%L_RADIO_NUM,argv[0],argv[1]);
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio id does not exist\n");
			else if(ret == RADIO_NO_BINDING_WLAN)
				vty_out(vty,"<error> radio is not binding wlan, please bind it first\n");
			else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
				vty_out(vty,"<error> radio doesn't bind wlan\n");
			else if(ret == RADIO_IS_DISABLE)
				vty_out(vty,"<error> radio is disable, please enable it first\n");
			else if(ret == RADIO_MODE_IS_11N)
				vty_out(vty,"<error> radio mode is not 11n,don't support this op\n");
			else
				vty_out(vty,"<error>  %d\n",ret);
		}
	else if(TYPE==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set %s %s successfully .\n",id,argv[0],argv[1]);
					if((count != 0)&&(TYPE == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		
		}
	return CMD_SUCCESS;

}

#else
DEFUN(set_radio_11n_ampdu_limit_func,
	  set_radio_11n_ampdu_limit_cmd,
	  "11n (ampdu|amsdu) limit VALUE",
	  "set ampdu|amsdu limit \n"
	  "ampdu:1024-65535 amsdu:2290-4096\n"
	 )
{
	unsigned int radio_id; 
	unsigned int limit;
	int ret;
	unsigned char type = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;	

    if (!strcmp(argv[0],"ampdu"))
	{
		type = 1;	
	}
	else if (!strcmp(argv[0],"amsdu"))
	{
		type = 2;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'ampdu' or 'amsdu'\n");
		return CMD_SUCCESS;
	}
	
	ret = parse_int_ID((char *)argv[1],&limit);
	if(ret != 0){
		vty_out(vty,"<error> input patameter error\n");
		return CMD_SUCCESS;
	}
	else if(ret == 0)
	{
    	if((type == 1)&&((limit<1024)||(limit>65535)))
    	{
    		vty_out(vty,"<error> input patameter error,ampdu limit should be 1024-65535\n");
    		return CMD_SUCCESS;
    	}
    	else if((type == 2)&&((limit<2290)||(limit>4096)))
    	{
            vty_out(vty,"<error> input patameter error,amsdu limit should be 2290-4096\n");
    		return CMD_SUCCESS;
    	}
	}

	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ret = dcli_radio_11n_set_ampdu_limit(localid,index,radio_id,limit,type,dcli_dbus_connection);

	if(ret == 0)
		vty_out(vty,"radio %d-%d set %s %s successfully .\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0],argv[1]);
	else if(ret == RADIO_ID_NOT_EXIST)
		vty_out(vty,"<error> radio id does not exist\n");
	else if(ret == RADIO_NO_BINDING_WLAN)
		vty_out(vty,"<error> radio is not binding wlan, please bind it first\n");
	else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
		vty_out(vty,"<error> radio doesn't bind wlan\n");
	else if(ret == RADIO_IS_DISABLE)
		vty_out(vty,"<error> radio is disable, please enable it first\n");
	else if(ret == RADIO_MODE_IS_11N)
		vty_out(vty,"<error> radio mode is not 11n,don't support this op\n");
	else
		vty_out(vty,"<error>  %d\n",ret);

	return CMD_SUCCESS;

}
#endif
#if _GROUP_POLICY
DEFUN(set_radio_11n_ampdu_subframe_func,
	  set_radio_11n_ampdu_subframe_cmd,
	  "11n (ampdu|amsdu) subframe VALUE",
	  "set ampdu|amsdu subframe \n"
	  "2-64\n"
	 )
{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int TYPE = 0;
	unsigned char subframe = 0;
	unsigned int limit = 0;
	unsigned char type = 0;

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 
    if (!strcmp(argv[0],"ampdu"))
	{
		type = 1;	
	}
	else if (!strcmp(argv[0],"amsdu"))
	{
		type = 2;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'ampdu' or 'amsdu'\n");
		return CMD_SUCCESS;
	}
	
	ret = parse_int_ID((char *)argv[1],&limit);
	if(ret != 0){
		vty_out(vty,"<error> input patameter error\n");
		return CMD_SUCCESS;
	}
	else if((ret == 0)&&((limit<2)||(limit>64))){
		vty_out(vty,"<error> input patameter error,limit should be 2-64\n");
		return CMD_SUCCESS;
	}

	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		TYPE = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		TYPE = 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		TYPE = 1;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	RadioList_Head = set_radio_11n_ampdu_subframe_cmd_11n(localid,index,dcli_dbus_connection,TYPE,id,limit,type,&count,&ret);

	if(TYPE==0)
		{
			if(ret == 0)
				vty_out(vty,"radio %d-%d set %s %s successfully .\n",id/L_RADIO_NUM,id%L_RADIO_NUM,argv[0],argv[1]);
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio id does not exist\n");
			else if(ret == RADIO_NO_BINDING_WLAN)
				vty_out(vty,"<error> radio is not binding wlan, please bind it first\n");
			else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
				vty_out(vty,"<error> radio doesn't bind wlan\n");
			else if(ret == RADIO_IS_DISABLE)
				vty_out(vty,"<error> radio is disable, please enable it first\n");
			else if(ret == RADIO_MODE_IS_11N)
				vty_out(vty,"<error> radio mode is not 11n,don't support this op\n");
			else
				vty_out(vty,"<error>  %d\n",ret);
		}
	else if(TYPE==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set %s %s successfully .\n",id,argv[0],argv[1]);
					if((count != 0)&&(TYPE == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		}

	return CMD_SUCCESS;

}

#else
DEFUN(set_radio_11n_ampdu_subframe_func,
	  set_radio_11n_ampdu_subframe_cmd,
	  "11n (ampdu|amsdu) subframe VALUE",
	  "set ampdu|amsdu subframe \n"
	  "2-64\n"
	 )
{
	unsigned int radio_id; 
	unsigned int limit;
	int ret;
	unsigned char type = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;	

    if (!strcmp(argv[0],"ampdu"))
	{
		type = 1;	
	}
	else if (!strcmp(argv[0],"amsdu"))
	{
		type = 2;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'ampdu' or 'amsdu'\n");
		return CMD_SUCCESS;
	}
	
	ret = parse_int_ID((char *)argv[1],&limit);
	if(ret != 0){
		vty_out(vty,"<error> input patameter error\n");
		return CMD_SUCCESS;
	}
	else if((ret == 0)&&((limit<2)||(limit>64))){
		vty_out(vty,"<error> input patameter error,limit should be 2-64\n");
		return CMD_SUCCESS;
	}

	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ret = dcli_radio_11n_set_ampdu_subframe(localid,index,radio_id,limit,type,dcli_dbus_connection);

	if(ret == 0)
		vty_out(vty,"radio %d-%d set %s %s successfully .\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0],argv[1]);
	else if(ret == RADIO_ID_NOT_EXIST)
		vty_out(vty,"<error> radio id does not exist\n");
	else if(ret == RADIO_NO_BINDING_WLAN)
		vty_out(vty,"<error> radio is not binding wlan, please bind it first\n");
	else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
		vty_out(vty,"<error> radio doesn't bind wlan\n");
	else if(ret == RADIO_IS_DISABLE)
		vty_out(vty,"<error> radio is disable, please enable it first\n");
	else if(ret == RADIO_MODE_IS_11N)
		vty_out(vty,"<error> radio mode is not 11n,don't support this op\n");
	else
		vty_out(vty,"<error>  %d\n",ret);

	return CMD_SUCCESS;

}
#endif
#if _GROUP_POLICY
/* zhangshu add for a-mpdu | a-msdu END */

DEFUN(set_radio_11n_puren_mixed_func,
	  set_radio_11n_puren_mixed_cmd,
	  "wlan VALUE workmode (puren|mixed)",
	  "set special wlan's work mode\n"
	  "set mixed mode\n"
	  "set puren mode\n"
	 )

{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned char policy = 0;
	unsigned char WLANID = 0;

	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 
	ret = parse_char_ID((char *)argv[0],&WLANID);
	if((ret != 0)||(WLANID<=0)){
		if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
			   vty_out(vty,"<error> input patameter error\n");
			}
		return CMD_SUCCESS;
	}else{}
	
	if (!strcmp(argv[1],"puren"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[1],"mixed"))
	{
		policy = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'puren' or 'mixed'\n");
		return CMD_SUCCESS;
	}

	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type = 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type = 1;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	RadioList_Head = set_radio_11n_puren_mixed_cmd_wlan_VALUE_workmode(localid,index,dcli_dbus_connection,type,id,policy,
		WLANID,&count,&ret);

	if(type==0)
		{
			if(ret == 0)
				vty_out(vty, 
						"radio %d-%d set wlan %s work mode for %s successfully.\n",
						id/L_RADIO_NUM,id%L_RADIO_NUM,
						argv[0],
						policy ? "puren" : "mixed");
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio id does not exist\n");
			else if(ret == RADIO_NO_BINDING_WLAN)
				vty_out(vty,"<error> radio is not binding wlan, please bind it first\n");
			else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
				vty_out(vty,"<error> radio doesn't bind wlan\n");
			else if(ret == RADIO_IS_DISABLE)
				vty_out(vty,"<error> radio is disable, please enable it first\n");
			else if(ret == RADIO_MODE_IS_11N)
				vty_out(vty,"<error> radio mode is not 11n,don't support this op\n");
			else
				vty_out(vty,"<error>  %d\n",ret);
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty, 
						"radio %d-%d set wlan %s work mode for %s successfully.\n",
						id/L_RADIO_NUM,id%L_RADIO_NUM,
						argv[0],
						policy ? "puren" : "mixed");
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		}
	return CMD_SUCCESS;
}

#else
/* zhangshu add for a-mpdu | a-msdu END */

DEFUN(set_radio_11n_puren_mixed_func,
	  set_radio_11n_puren_mixed_cmd,
	  "wlan VALUE workmode (puren|mixed)",
	  CONFIG_STR
	  "set special wlan's work mode\n"
	  "set special wlan's work mode\n"
	  "set puren mode\n"
	  "set mixed mode\n"
	 )


{
	unsigned int radio_id; 
	int ret;
	unsigned char policy = 0;
	unsigned char WLANID;
	ret = parse_char_ID((char *)argv[0],&WLANID);
	if((ret != 0)||(WLANID<=0)){
		if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
			   vty_out(vty,"<error> input patameter error\n");
			}
		return CMD_SUCCESS;
	}else{}
	
	if (!strcmp(argv[1],"puren"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[1],"mixed"))
	{
		policy = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'puren' or 'mixed'\n");
		return CMD_SUCCESS;
	}

	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ret = dcli_radio_11n_set_mixed_puren_switch(localid,index,policy,WLANID,radio_id,dcli_dbus_connection);

	if(ret == 0)
		vty_out(vty, 
				"radio %d-%d set wlan %s work mode for %s successfully.\n",
				radio_id/L_RADIO_NUM, radio_id%L_RADIO_NUM,
				argv[0],
				policy ? "puren" : "mixed");
	else if(ret == RADIO_ID_NOT_EXIST)
		vty_out(vty,"<error> radio id does not exist\n");
	else if(ret == RADIO_NO_BINDING_WLAN)
		vty_out(vty,"<error> radio %d-%d is not binding wlan %s, please bind it first\n", radio_id/L_RADIO_NUM, radio_id%L_RADIO_NUM, argv[0]);
	else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
		vty_out(vty,"<error> radio %d-%d doesn't bind wlan %s\n", radio_id/L_RADIO_NUM, radio_id%L_RADIO_NUM, argv[0]);
	else if(ret == RADIO_IS_DISABLE)
		vty_out(vty,"<error> radio %d-%d is disable, please enable it first\n", radio_id/L_RADIO_NUM, radio_id%L_RADIO_NUM);
	else if(ret == RADIO_MODE_IS_11N)
			vty_out(vty,"<error> radio %d-%d mode is not 11n,don't support this op\n", radio_id/L_RADIO_NUM, radio_id%L_RADIO_NUM);
	else if(ret == WID_DBUS_ERROR) //fengwenchao add 20120716 for autelan-3057
		vty_out(vty,"<error> now radio %d-%d mode is an or gn, belong to puren,you can set it to mixed\n", radio_id/L_RADIO_NUM, radio_id%L_RADIO_NUM);
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);

	return CMD_SUCCESS;

}
#endif
#if _GROUP_POLICY

/*nl add 20100312*/
DEFUN(set_tx_chainmask_cmd_func,
	  set_tx_chainmask_cmd,
	  "set tx_chainmask  LIST (enable|disable)",
	  "set tx_chainmask list\n"
	  "Radio tx_chainmask value\n"
	 )
{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int type = 0;
	unsigned int policy = 0;
	unsigned char hex_id = 0;
	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 
	
	ret = parse_radio_tx_chainmask_list((char*)argv[0],&hex_id);

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
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		vty_out(vty,"<error> input parameter only should be 0,1,2, one or more of them,such as 1,2 or 0,2.\n");
		return CMD_SUCCESS;
	}
	
	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type = 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type = 1;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	RadioList_Head = set_tx_chainmask_cmd_set_tx_chainmask_LIST_enable_disable(localid,index,dcli_dbus_connection,type,id,
		hex_id,policy,&count,&ret);

	if(type==0)
		{
			if(ret == -1)
				{
					cli_syslog_info("<error> failed get reply.\n");
				}
			else if(ret == 0)
					vty_out(vty,"set radio tx_chainmask %s successfully .\n",argv[0]);
			else if(ret == RADIO_ID_NOT_EXIST)
					vty_out(vty,"<error> radio id does not exist\n");
			else if(ret == RADIO_NOT_SUPPORT_COMMAND)
					vty_out(vty,"<error> radio not support this command\n");
			else
					vty_out(vty,"<error>  %d\n",ret);
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set radio tx_chainmask %s successfully .\n",id,argv[0]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		}


	return CMD_SUCCESS;

}

#else
/*nl add 20100312*/
DEFUN(set_tx_chainmask_cmd_func,
	  set_tx_chainmask_cmd,
	  "set tx_chainmask  LIST (enable|disable)",
	  "set tx_chainmask list\n"
	  "Radio tx_chainmask value\n"
	 )
{
	unsigned int radio_id; 
	unsigned int wlanid = 0; 
	int policy = 0;
	int ret = 0;
	unsigned char hex_id = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;	
	
	ret = parse_radio_tx_chainmask_list((char*)argv[0],&hex_id);

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
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		vty_out(vty,"<error> input parameter only should be 0,1,2, one or more of them,such as 1,2 or 0,2.\n");
		return CMD_SUCCESS;
	}
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ret = radio_set_method_functiontwo(localid,index,radio_id,wlanid,hex_id,policy,NULL,WID_DBUS_RADIO_TX_CHAINMASK_SET_CMD,dcli_dbus_connection);
	
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0)
		vty_out(vty,"set radio tx_chainmask %s successfully .\n",argv[0]);
	else if(ret == RADIO_ID_NOT_EXIST)
		vty_out(vty,"<error> radio id does not exist\n");
	else if(ret == RADIO_NOT_SUPPORT_COMMAND)
		vty_out(vty,"<error> radio not support this command\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
//	dbus_message_unref(reply);
	return CMD_SUCCESS;

}
#endif

#if _GROUP_POLICY

/*wuwl add 20100313*/
DEFUN(set_radio_11n_channel_offset_func,
	  set_radio_11n_channel_offset_cmd,
	  "channel offset (up|down)",
	  "set channel policy \n"
	  "11n Radio channel offset\n"
	 )
{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned char policy = 0;
	unsigned int type = 0;
	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
		
	 
	if (!strcmp(argv[0],"up"))
	{
		policy = 1; 
	}
	/*else if (!strcmp(argv[0],"none"))
	{
		policy = 0; 
	}*/
	else if (!strcmp(argv[0],"down"))
	{
		policy = -1; 
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'enable' or 'down'\n");
		return CMD_SUCCESS;
	}

	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		type = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		type = 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		type = 1;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	RadioList_Head = set_radio_11n_channel_offset_cmd_channel_offset(localid,index,dcli_dbus_connection,type,id,
		policy,&count,&ret);

	if(type==0)
		{
			if(ret == 0)
				vty_out(vty,"radio %d-%d set channel offset %s successfully .\n",id/L_RADIO_NUM,id%L_RADIO_NUM,argv[0]);
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio id does not exist\n");
			else if(ret == RADIO_NO_BINDING_WLAN)
				vty_out(vty,"<error> radio is not binding wlan, please bind it first\n");
			else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
				vty_out(vty,"<error> radio doesn't bind wlan\n");
			else if(ret == RADIO_IS_DISABLE)
				vty_out(vty,"<error> radio is disable, please enable it first\n");
			else if(ret == RADIO_MODE_IS_11N)
				vty_out(vty,"<error> radio mode is not 11n,don't support this op\n");
			else if(ret == RADIO_HT_IS_NOT_40)
				vty_out(vty,"<error> radio channal bandwidth is not 40,don't support this op\n");
			else
				vty_out(vty,"<error>  %d\n",ret);
		}
	else if(type==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set channel offset %s successfully .\n",id,argv[0]);
					if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
		}

	return CMD_SUCCESS;

}

#else
/*wuwl add 20100313*/
DEFUN(set_radio_11n_channel_offset_func,
	  set_radio_11n_channel_offset_cmd,
	  "channel offset (up|down)",
	  "set channel policy \n"
	  "11n Radio channel offset\n"
	  "11n Radio channel offset up\n"
	  "11n Radio channel offset down\n"
	 )
{
	unsigned int radio_id = 0; 
	char policy = 0;
	int ret = 0;
	unsigned int max_channel = 0;
	unsigned int min_channel = 0;
	unsigned char current_channel = 0;
	if (!strcmp(argv[0],"up"))
	{
		policy = 1; 
	}
	else if (!strcmp(argv[0],"down"))
	{
		policy = -1; 
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'enable' or 'down'\n");
		return CMD_SUCCESS;
	}

	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	dcli_radio_11n_channel_offset(localid,index,radio_id,policy,&ret,&max_channel,&min_channel,&current_channel,dcli_dbus_connection);

	if(ret == 0)
		vty_out(vty,"radio %d-%d set channel offset %s successfully .\n",radio_id/L_RADIO_NUM,radio_id%L_RADIO_NUM,argv[0]);
	else if(ret == RADIO_ID_NOT_EXIST)
		vty_out(vty,"<error> radio id does not exist\n");
	else if(ret == RADIO_NO_BINDING_WLAN)
		vty_out(vty,"<error> radio is not binding wlan, please bind it first\n");
	else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
		vty_out(vty,"<error> radio doesn't bind wlan\n");
	else if(ret == RADIO_IS_DISABLE)
		vty_out(vty,"<error> radio is disable, please enable it first\n");
	else if(ret == RADIO_MODE_IS_11N)
		vty_out(vty,"<error> radio mode is not 11n,don't support this op\n");
	else if(ret == RADIO_HT_IS_NOT_40)
		vty_out(vty,"<error> radio channel bandwidth is not 40,don't support this op\n");
	else if(ret == CHANNEL_CWMODE_HT40){
	      if(1 == policy){
	       vty_out(vty,"<error>the current radio channel is %d which is larger than the max channel %d ,you are not allowed to set channel offset up!! Please turn down channel\n",current_channel,max_channel);
		   }
		  else if(-1 == policy){
	       vty_out(vty,"<error>the current radio channel is %d which is less than the min channel %d ,you are not allowed to set channel offset down!! Please turn up channel\n",current_channel,min_channel);
		  }
   }
	else
		vty_out(vty,"<error>  %d\n",ret);

	return CMD_SUCCESS;

}
#endif
#if 0
DEFUN(set_tx_chainmask_cmd_v2_func,
	  set_tx_chainmask_v2_cmd,
	  "tx_chainmask (1.0.0|1.0.1|1.1.0|1.1.1)",
	  "set txantenna list\n"
	  "Radio tx_chainmask value\n"
	 )
{
	unsigned int radio_id; 
	unsigned char policy = 0;
	int ret = 0;

	if (!strcmp(argv[0],"1.0.0"))
	{
		policy = 1; 
	}
	else if (!strcmp(argv[0],"1.1.0"))
	{
		policy = 2; 
	}
	else if (!strcmp(argv[0],"1.0.1"))
	{
		policy = 3; 
	}
	else if (!strcmp(argv[0],"1.1.1"))
	{
		policy = 4; 
	}
	else
	{
		vty_out(vty,"<error> input patameter only with '1.0.0','1.0.1','1.1.0' or '1.1.1'\n");
		return CMD_SUCCESS;
	}
	
	
	int index = 0;
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		radio_id = (int)vty->index_sub;
	}
	ret = radio_set_method_tx_chainmask(localid,index,radio_id,policy,dcli_dbus_connection);
	
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0)
		vty_out(vty,"set radio tx_chainmask %s successfully .\n",argv[0]);
	else if(ret == RADIO_ID_NOT_EXIST)
		vty_out(vty,"<error> radio id does not exist\n");
	else if(ret == RADIO_NOT_SUPPORT_COMMAND)
		vty_out(vty,"<error> radio not support this command\n");
	else if(ret == RADIO_MODE_IS_11N)
		vty_out(vty,"<error> radio mode is not 11N ,don't support this command\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
//	dbus_message_unref(reply);
	return CMD_SUCCESS;

}
#endif
#if _GROUP_POLICY
/* zhangshu add for rx_chainmask, 2010-10-09*/
DEFUN(set_tx_chainmask_cmd_v2_func,
	  set_tx_chainmask_v2_cmd,
	  "(tx_chainmask|rx_chainmask) (1.0.0|0.1.0|1.1.0|0.0.1|1.0.1|0.1.1|1.1.1)",
	  "set txantenna list\n"
	  "Radio tx_chainmask/rx_chainmask value\n"
	 )
{
	int i = 0;
	int ret = 0;
	int count = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int TYPE = 0;
	unsigned char policy = 0;
	unsigned char type = 0;
	unsigned char chainmask_num = 0;
	
	struct RadioList *RadioList_Head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
	
	 


    /* check para 1 */
    if (!strcmp(argv[0],"tx_chainmask"))
	{
		type = 1; 
	}
	else if (!strcmp(argv[0],"rx_chainmask"))
	{
		type = 2; 
	}
	else
	{
	    vty_out(vty,"<error> input patameter only with 'tx_chainmask' or 'rx_chainmask'\n");
		return CMD_SUCCESS;
	}

    /* check para 2 */
	if (!strcmp(argv[1],"0.0.1"))
	{
		policy = 1; 
	}
	else if (!strcmp(argv[1],"0.1.0"))
	{
		policy = 2; 
	}
	else if (!strcmp(argv[1],"0.1.1"))
	{
		policy = 3; 
	}
	else if (!strcmp(argv[1],"1.0.0"))
	{
		policy = 4; 
	}
	else if (!strcmp(argv[1],"1.0.1"))
	{
		policy = 5; 
	}
	else if (!strcmp(argv[1],"1.1.0"))
	{
		policy = 6; 
	}
	else if (!strcmp(argv[1],"1.1.1"))
	{
		policy = 7; 
	}
	else
	{
		vty_out(vty,"<error> input patameter only with '0.0.1','0.1.0','0.1.1','1.0.0','1.0.1','1.1.0' or '1.1.1'\n");
		return CMD_SUCCESS;
	}

	if(vty->node == RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
    }else if(vty->node == AP_GROUP_RADIO_NODE){
		index = 0;			
		id = (unsigned)vty->index;
		TYPE = 1;
		vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
	}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		id = (unsigned)vty->index_sub;
		TYPE = 1;
	}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
        id = (unsigned)vty->index_sub;
		TYPE = 1;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	RadioList_Head = set_tx_chainmask_v2_cmd_tx_chainmask_rx_chainmask(localid,index,dcli_dbus_connection,TYPE,id,
		policy,type,&count,&ret);

	if(TYPE==0)
		{
			if(ret == -1){
				cli_syslog_info("<error> failed get reply.\n");
			}
			else if(ret == 0)
				vty_out(vty,"set radio %s %s successfully .\n",argv[0],argv[1]);
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio id does not exist\n");
			else if(ret == RADIO_NOT_SUPPORT_COMMAND)
				vty_out(vty,"<error> radio not support this command\n");
			else if(ret == RADIO_MODE_IS_11N)
				vty_out(vty,"<error> radio mode is not 11N ,don't support this command\n");
			else if(ret == RADIO_CHAINMASK_NUM_1)
				vty_out(vty,"<error> radio chainmask number is 1, don't support this value.\n");
			else if(ret == RADIO_CHAINMASK_NUM_2)
				vty_out(vty,"<error> radio chainmask number is 2, don't support this value.\n");
			else
				vty_out(vty,"<error>  %d\n",ret);			
		}
	else if(TYPE==1)
		{
			if(ret == 0)
				{
					vty_out(vty,"group %d set radio %s %s successfully .\n",id,argv[0],argv[1]);
					if((count != 0)&&(TYPE == 1)&&(RadioList_Head!=NULL))
						{
							vty_out(vty,"radio ");					
							for(i=0; i<count; i++)
							{
								if(Radio_Show_Node == NULL)
									Radio_Show_Node = RadioList_Head->RadioList_list;
								else 
									Radio_Show_Node = Radio_Show_Node->next;
								if(Radio_Show_Node == NULL)
									break;
								vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
							}
							vty_out(vty," failed.\n");
							dcli_free_RadioList(RadioList_Head);
						}
				}
			else if (ret == GROUP_ID_NOT_EXIST)
				vty_out(vty,"<error> group id does not exist\n");
			else if (ret == -1)
				cli_syslog_info("<error> failed get reply.\n");				
		}
//	dbus_message_unref(reply);
	return CMD_SUCCESS;

}

#else
/* zhangshu add for rx_chainmask, 2010-10-09*/
DEFUN(set_tx_chainmask_cmd_v2_func,
	  set_tx_chainmask_v2_cmd,
	  "(tx_chainmask|rx_chainmask) (1.0.0|0.1.0|1.1.0|0.0.1|1.0.1|0.1.1|1.1.1)",
	  "set txantenna list\n"
	  "Radio tx_chainmask/rx_chainmask value\n"
	 )
{
	unsigned int radio_id; 
	unsigned char policy = 0;
	unsigned char type = 0;
	int ret = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;

    /* check para 1 */
    if (!strcmp(argv[0],"tx_chainmask"))
	{
		type = 1; 
	}
	else if (!strcmp(argv[0],"rx_chainmask"))
	{
		type = 2; 
	}
	else
	{
	    vty_out(vty,"<error> input patameter only with 'tx_chainmask' or 'rx_chainmask'\n");
		return CMD_SUCCESS;
	}

    /* check para 2 */
	if (!strcmp(argv[1],"0.0.1"))
	{
		policy = 1; 
	}
	else if (!strcmp(argv[1],"0.1.0"))
	{
		policy = 2; 
	}
	else if (!strcmp(argv[1],"0.1.1"))
	{
		policy = 3; 
	}
	else if (!strcmp(argv[1],"1.0.0"))
	{
		policy = 4; 
	}
	else if (!strcmp(argv[1],"1.0.1"))
	{
		policy = 5; 
	}
	else if (!strcmp(argv[1],"1.1.0"))
	{
		policy = 6; 
	}
	else if (!strcmp(argv[1],"1.1.1"))
	{
		policy = 7; 
	}
	else
	{
		vty_out(vty,"<error> input patameter only with '0.0.1','0.1.0','0.1.1','1.0.0','1.0.1','1.1.0' or '1.1.1'\n");
		return CMD_SUCCESS;
	}

	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ret = radio_set_method_chainmask(localid,index,radio_id,policy,type,dcli_dbus_connection);
	
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0)
		vty_out(vty,"set radio %s %s successfully .\n",argv[0],argv[1]);
	else if(ret == RADIO_ID_NOT_EXIST)
		vty_out(vty,"<error> radio id does not exist\n");
	else if(ret == RADIO_NOT_SUPPORT_COMMAND)
		vty_out(vty,"<error> radio not support this command\n");
	else if(ret == RADIO_MODE_IS_11N)
		vty_out(vty,"<error> radio mode is not 11N ,don't support this command\n");
	else if(ret == RADIO_CHAINMASK_NUM_1)
		vty_out(vty,"<error> radio chainmask number is 1, don't support this value.\n");
	else if(ret == RADIO_CHAINMASK_NUM_2)
		vty_out(vty,"<error> radio chainmask number is 2, don't support this value.\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
//	dbus_message_unref(reply);
	return CMD_SUCCESS;

}
#endif
#if _GROUP_POLICY
/* zhangshu add END, 2010-10-09 */

DEFUN(set_radio_txpowerstep_cmd_func,
	  set_radio_txpowerstep_cmd,
	  "txpowerstep TXPOWER",
	  "set radio txpower step\n"
	  "Radio txpowerstep value\n"
	 )
	 {
		
		int i = 0;
		int ret = 0;
		int count = 0;
		int index = 0;
		int localid = 1;
        int slot_id = HostSlotId;
		unsigned int id = 0;
		unsigned int type = 0;
		unsigned short txpowerstep = 0;
		struct RadioList *RadioList_Head = NULL;
		struct RadioList *Radio_Show_Node = NULL;
		
		 

		ret = parse_short_ID((char *)argv[0],&txpowerstep);
		//printf("@@@@@@@@@txpowerstep = %d@@@@@@@@@@\n",txpowerstep);

		if(txpowerstep == 0)
		{
			vty_out(vty,"txpowerstep should > 0!!\n");
			return CMD_SUCCESS;
		}
		if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
			    vty_out(vty,"<error> unknown id format\n");
			}
			return CMD_SUCCESS;
	    }	
		if(vty->node == RADIO_NODE){
			index = 0;			
			id = (unsigned)vty->index;
	    }
		else if(vty->node == HANSI_RADIO_NODE){
			index = vty->index; 		
			localid = vty->local;
            slot_id = vty->slotindex;
			id = (unsigned)vty->index_sub;
	    }else if (vty->node == LOCAL_HANSI_RADIO_NODE){
            index = vty->index;
            localid = vty->local;
            slot_id = vty->slotindex;
			id = (unsigned)vty->index_sub;
        }else if(vty->node == AP_GROUP_RADIO_NODE){
			index = 0;			
			id = (unsigned)vty->index;
			type = 1;
			vty_out(vty,"*******type == 1*** AP_GROUP_WTP_NODE*****\n");
		}else if(vty->node == HANSI_AP_GROUP_RADIO_NODE){
			index = vty->index; 		
			localid = vty->local;
            slot_id = vty->slotindex;
			id = (unsigned)vty->index_sub;
			type = 1;
		}else if (vty->node == LOCAL_HANSI_AP_GROUP_RADIO_NODE){
            index = vty->index;
            localid = vty->local;
            slot_id = vty->slotindex;
            id = (unsigned)vty->index_sub;
			type = 1;
        }
        DBusConnection *dcli_dbus_connection = NULL;
        ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

		RadioList_Head = set_radio_txpowerstep_cmd_txpowerstep(localid,index,dcli_dbus_connection,type,id,txpowerstep,
			&count,&ret);

		if(type==0)
			{
				if(ret == -1)
					{
		   				cli_syslog_info("<error> failed get reply.\n");
	    			}
	   			else if(ret == 0)
		       			vty_out(vty,"set radio txpower step %s successfully .\n",argv[0]);
				else if(ret == RADIO_ID_NOT_EXIST)
						vty_out(vty,"<error> radio id does not exist\n");
				else if(ret == RADIO_NO_BINDING_WLAN)
						vty_out(vty,"<error> this radio is not binding wlan,binding wlan first.\n");
			}
		else if(type==1)
			{
				if(ret == 0)
					{
						vty_out(vty,"group %d set radio txpower step %s successfully .\n",id,argv[0]);
						if((count != 0)&&(type == 1)&&(RadioList_Head!=NULL))
							{
								vty_out(vty,"radio ");					
								for(i=0; i<count; i++)
								{
									if(Radio_Show_Node == NULL)
										Radio_Show_Node = RadioList_Head->RadioList_list;
									else 
										Radio_Show_Node = Radio_Show_Node->next;
									if(Radio_Show_Node == NULL)
										break;
									vty_out(vty,"%d ",Radio_Show_Node->RadioId);					
								}
								vty_out(vty," failed.\n");
								dcli_free_RadioList(RadioList_Head);
							}
					}
				else if (ret == GROUP_ID_NOT_EXIST)
			  		vty_out(vty,"<error> group id does not exist\n");
			}
		return CMD_SUCCESS;
     }

#else
/* zhangshu add END, 2010-10-09 */

DEFUN(set_radio_txpowerstep_cmd_func,
	  set_radio_txpowerstep_cmd,
	  "txpowerstep TXPOWER",
	  "set radio txpower step\n"
	  "Radio txpowerstep value\n"
	 )
{
	unsigned int radio_id; 
	unsigned short txpowerstep;
	int ret = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	ret = parse_short_ID((char *)argv[0],&txpowerstep);
	if(txpowerstep == 0)
	{
		vty_out(vty,"txpowerstep should > 0!!\n");
		return CMD_SUCCESS;
	}
	if(ret != WID_DBUS_SUCCESS){
	        if(ret == WID_ILLEGAL_INPUT){
	        		vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
	        }
		else{
			vty_out(vty,"<error> unknown id format\n");
		}
		return CMD_SUCCESS;
	}	
	if(vty->node == RADIO_NODE){
		index = 0;			
		radio_id = (int)vty->index;
	}
	else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radio_id = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ret = radio_set_txpower_step(localid,index,radio_id,txpowerstep,dcli_dbus_connection);
	
	if(ret == -1) {
		   cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0)
		vty_out(vty,"set radio txpower step %s successfully .\n",argv[0]);
	else if(ret == RADIO_ID_NOT_EXIST)
		vty_out(vty,"<error> radio id does not exist\n");
	else if(ret == RADIO_NO_BINDING_WLAN)
		vty_out(vty,"<error> this radio is not binding wlan,binding wlan first.\n");
	return CMD_SUCCESS;
     }
#endif
/*fengwenchao add 20120221 for RDIR-25*/
DEFUN(set_radio_wlan_limit_rssi_access_sta_cmd_func,
	set_radio_wlan_limit_rssi_access_sta_cmd,
	"set radio wlan WLANID access sta limit rssi RSSI",
	"set radio wlan limit rssi access sta\n"
	"radio\n"
	"wlan\n"
	"wlanid:range 1 to 128\n"
	"access sta\n"
	"access sta\n"
	"acess sta limit\n"
	"acess sta limit rssi\n"
	"rssi:range 0 to 95  0:means close this function\n"
	)
{
	unsigned int radioid = 0;
	unsigned char wlanid = 0;
	unsigned char rssi =0;
	int ret = 0;
	int index = 0;
	int localid = 1;
    	int slot_id = HostSlotId;
	ret = parse_char_ID((char *)argv[0],&wlanid);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}
	if(wlanid >= WLAN_NUM ||wlanid == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	ret = parse_char_ID((char *)argv[1],&rssi);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(rssi > 95 ||rssi < 0){
		vty_out(vty,"<error> RSSI should be 0 to 95\n");
		return CMD_SUCCESS;
	}	

	if(vty->node == RADIO_NODE){
		index = 0;			
		radioid = (unsigned int)vty->index;
	}
	else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 	
		localid = vty->local;
        		slot_id = vty->slotindex;
		radioid = (unsigned int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        		index = vty->index;
        		localid = vty->local;
        		slot_id = vty->slotindex;
		radioid = (unsigned int)vty->index_sub;
    	}	

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ret = set_radio_wlan_limit_rssi_access_sta_set(localid,index,radioid,wlanid,rssi,dcli_dbus_connection);

	if(ret == -1) 
	{
		vty_out(vty,"<error> failed get reply.\n");
	 }
	 else if(ret == 0)
		 vty_out(vty,"set radio wlan %d access sta limit rssi %d.\n",wlanid,rssi);
	 else if(ret == WLAN_BE_DISABLE)
	 {
	 	 vty_out(vty,"set radio wlan %d access sta limit rssi %d.\n",wlanid,rssi);
	  	 vty_out(vty,"Warning:now wlan is disable ,ap could not success\n");
	 }
	 else if(ret == RADIO_ID_NOT_EXIST)
		 vty_out(vty,"<error> radio id does not exist\n");
	  else if(ret == WTP_ID_NOT_EXIST)
		  vty_out(vty,"<error> wtp id does not exist\n");
	  else if(ret ==WLAN_ID_NOT_EXIST)
		  vty_out(vty,"<error> wlan id does not exist\n");
	  else if(ret == BSS_NOT_EXIST)
		   vty_out(vty,"<error> bss id does not exist\n");
	  else if(ret == Wlan_IF_NOT_BE_BINDED)
		    vty_out(vty,"<error> wlan is not bind by this radio\n");
	  else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	  {
		  vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	  }
		return CMD_SUCCESS;    	
}
/*fengwenchao add end*/

/*wuwl add for REQUIREMENTS-340*/
DEFUN(set_ap_uni_muti_bro_cast_isolation_set_func,
	  set_ap_uni_muti_bro_cast_isolation_set_cmd,
	  "wlan WLANID (unicast|multicast_broadcast|unicast_and_multicast_broadcast|wifi) isolation (enable|disable)",
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
	unsigned int radioid = 0;
	unsigned char policy = 0;
	unsigned int unicast = 0;
	unsigned int rate = 0;
	int ret;
	unsigned char wlanid = 0;
	int localid = 1;
	int slot_id = HostSlotId;
	ret = parse_char_ID((char *)argv[0],&wlanid);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}
	if(wlanid < 1 || wlanid >= WLAN_NUM){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	if (!strcmp(argv[1],"unicast"))
	{
		unicast = 1;	
	}
	else if (!strcmp(argv[1],"multicast_broadcast"))
	{
		unicast = 0;	
	}
	else if(!strcmp(argv[1],"unicast_and_multicast_broadcast")){
		unicast = 2;	
	}
	else if(!strcmp(argv[1],"wifi")){
		unicast = 3;
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'unicast' or 'multicast_broadcast'\n");
		return CMD_WARNING;
	}
	if (!strcmp(argv[2],"enable"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[2],"disable"))
	{
		policy = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'enable' or 'disable'\n");
		return CMD_WARNING;
	}
	if((1 == unicast)&&(argc > 3)){
		vty_out(vty,"<error> only muti and broadcast can set rate.\n");
		return CMD_WARNING;
	}
	if (4 == argc) {
		rate = strtoul((char *)argv[3], NULL, 10);
		if (rate == 10 ||rate == 20||rate == 55||rate == 60||rate == 90||rate == 110||rate == 120||rate == 180||rate == 240||rate == 360||rate == 480||rate == 540)
		{
		}else{
			vty_out(vty,"%% Invalid rate: %s !\n", argv[3]);
			vty_out(vty,"10 20 55 60 90 110 120 180 240 360 480 540.\n");
			vty_out(vty,"if you want to set rate 5.5M,you should input 55.\n");
			return CMD_WARNING;
		}
	}
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;			
		radioid = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index; 		
		radioid = 0;
		localid = vty->local;
        slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_NODE){
		index = vty->index; 		
		radioid = 0;
		localid = vty->local;
        slot_id = vty->slotindex;
	}else if(vty->node == RADIO_NODE){
		index = 0; 		
		radioid = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		radioid = (int)vty->index_sub;
		localid = vty->local;
        slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_RADIO_NODE){
		index = vty->index; 		
		radioid = (int)vty->index_sub;
		localid = vty->local;
        slot_id = vty->slotindex;
	}

    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_UNI_MUTI_BR_CAST_ISOLATION_SW_AND_RATE_SET);
																			
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radioid,
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
		if(4 == argc)
			vty_out(vty," set radio%d-%d.%d %s isolation %s rate %s successfully\n",radioid/L_RADIO_NUM,radioid%L_RADIO_NUM,wlanid,argv[1],argv[2],argv[3]);
		else
			vty_out(vty," set radio%d-%d.%d %s isolation %s successfully\n",radioid/L_RADIO_NUM,radioid%L_RADIO_NUM,wlanid,argv[1],argv[2]);
	}
	else if (ret == WTP_ID_NOT_EXIST)
		vty_out(vty,"<error> wtp id does not exist\n");
	else if(ret == RADIO_NO_BINDING_WLAN)
		vty_out(vty,"<error> radio %d-%d not bind wlan %d.\n",radioid/L_RADIO_NUM,radioid%L_RADIO_NUM,wlanid);
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan %d does not exist\n",wlanid);
	else if(ret == RADIO_SUPPORT_RATE_EXIST)
		vty_out(vty,"<error> radio %d-%d does support rate %d.\n",radioid/L_RADIO_NUM,radioid%L_RADIO_NUM,rate);
	else if(ret == BSS_NOT_EXIST)
		vty_out(vty,"<error> bss not exsit.");
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
/*wuwl add for REQUIREMENTS-340*/
DEFUN(set_ap_muti_bro_cast_rate_set_func,
	  set_ap_muti_bro_cast_rate_set_cmd,
	  "wlan WLANID multicast_broadcast_rate RATE",
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
	unsigned int radioid = 0;
	unsigned char policy = 0;
	unsigned int unicast = 0;
	unsigned int rate = 0;
	int ret;
	int localid = 1;
	int slot_id = HostSlotId;
	unsigned char wlanid = 0;
	ret = parse_char_ID((char *)argv[0],&wlanid);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}
	if(wlanid < 1 || wlanid >= WLAN_NUM){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	ret = parse_int_ID((char *)argv[1],&rate);
	if(ret == 0)
	{
		if (rate == 10 ||rate == 20||rate == 55||rate == 60||rate == 90||rate == 110||rate == 120||rate == 180||rate == 240||rate == 360||rate == 480||rate == 540)
		{
		}else{
			vty_out(vty,"%% Invalid rate: %s !\n", argv[1]);
			vty_out(vty,"10 20 55 60 90 110 120 180 240 360 480 540.\n");
			vty_out(vty,"if you want to set rate 5.5M,you should input 55.\n");
			return CMD_WARNING;
		}
	}
	else
	{
		vty_out(vty,"<error> unkown format\n");
		return CMD_SUCCESS;
	}
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;			
		radioid = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index; 		
		radioid = 0;
		localid = vty->local;
        slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_NODE){
		index = vty->index; 		
		radioid = 0;
		localid = vty->local;
        slot_id = vty->slotindex;
	}else if(vty->node == RADIO_NODE){
		index = 0; 		
		radioid = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		radioid = (int)vty->index_sub;
		localid = vty->local;
        slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_RADIO_NODE){
		index = vty->index; 		
		radioid = (int)vty->index_sub;
		localid = vty->local;
        slot_id = vty->slotindex;
	}
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_MUTI_BR_CAST_RATE_SET);
																			
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radioid,
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
		vty_out(vty," set radio%d-%d.%d rate %d successfully\n",radioid/L_RADIO_NUM,radioid%L_RADIO_NUM,wlanid,rate);
	}
	else if (ret == WTP_ID_NOT_EXIST)
		vty_out(vty,"<error> wtp id does not exist\n");
	else if(ret == RADIO_NO_BINDING_WLAN)
		vty_out(vty,"<error> radio %d-%d not bind wlan %d.\n",radioid/L_RADIO_NUM,radioid%L_RADIO_NUM,wlanid);
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan %d does not exist\n",wlanid);
	else if(ret == RADIO_SUPPORT_RATE_EXIST)
		vty_out(vty,"<error> radio %d-%d does support rate %d.\n",radioid/L_RADIO_NUM,radioid%L_RADIO_NUM,rate);
	else if(ret == BSS_NOT_EXIST)
		vty_out(vty,"<error> bss not exsit.");
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

/*wuwl add for REQUIREMENTS-340*/
DEFUN(set_ap_not_response_sta_probe_request_func,
	  set_ap_not_response_sta_probe_request_cmd,
	  "wlan WLANID no response to sta probe request (enable|disable)",
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
	unsigned int radioid = 0;
	unsigned int policy = 0;
	unsigned char wlanid = 0;
	int ret;
	int localid = 1;
	int slot_id = HostSlotId;
	ret = parse_char_ID((char *)argv[0],&wlanid);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}
	if(wlanid < 1 || wlanid >= WLAN_NUM){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
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

	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;			
		radioid = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index; 		
		radioid = 0;
		localid = vty->local;
        slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_NODE){
		index = vty->index; 		
		radioid = 0;
		localid = vty->local;
        slot_id = vty->slotindex;
	}else if(vty->node == RADIO_NODE){
		index = 0; 		
		radioid = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 		
		radioid = (int)vty->index_sub;
		localid = vty->local;
        slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_RADIO_NODE){
		index = vty->index; 		
		radioid = (int)vty->index_sub;
		localid = vty->local;
        slot_id = vty->slotindex;
	}
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_NO_RESPONSE_TO_STA_PROBLE_REQUEST);
																			
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radioid,
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
		vty_out(vty," set wtp %d-%d no response to sta probe request %s successfully\n",radioid/L_RADIO_NUM,radioid%L_RADIO_NUM,argv[0]);
	}
	else if (ret == WTP_ID_NOT_EXIST)
		vty_out(vty,"<error> wtp id does not exist\n");
	else if(ret == RADIO_NO_BINDING_WLAN)
		vty_out(vty,"<error> radio %d-%d not bind wlan %d.\n",radioid/L_RADIO_NUM,radioid%L_RADIO_NUM,wlanid);
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan %d does not exist\n",wlanid);
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
DEFUN(set_radio_servive_timer_func,
		  set_radio_servive_timer_func_cmd,
		  "set radio (start|stop) service at TIME (once|cycle) .WEEKDAYS",
		  "sta radio service control timer\n"
		  "start service or stop service\n"
		  "time like 21:12:32 \n"
		  "once or cycle\n"
		  "weekdays you want (like Sun Mon Tue Wed Thu Fri Sat or hebdomad)\n"
		  "eg: set radio start service at 21:12:34 once mon tue sat"
	 )
{	
	int ret;
	unsigned int radioid = 0;
	unsigned int i = 0;
    int policy = 0;	
    int is_once = 0;
	int time;
	int index;
	int localid = 1;
    int slot_id = HostSlotId;
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
	if(time == 0){
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
	
	if(vty->node == RADIO_NODE){
		index = 0;			
		radioid = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = (int)vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		radioid = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radioid = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ret = dcli_radio_service_control_timer(localid,index,policy,radioid,is_once,wday,time,dcli_dbus_connection);
	if(ret == 0)
		vty_out(vty,"set radio %s service at TIME %s successfully\n",argv[0],argv[1]);
	else if(ret == RADIO_SERVICE_CONTROL_BE_USED){
		vty_out(vty,"Radio Service %stimer Control be used,please disable it first.\n",argv[0]);	
		}
	else if(ret == RADIO_ID_NOT_EXIST){
		vty_out(vty,"Radio ID not exist\n");
		}
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	return CMD_SUCCESS; 

}
DEFUN(set_radio_timer_able_func,
		  set_radio_timer_able_cmd,
		  "set radio (starttimer|stoptimer) (enable|disable)",
		  "sta ip mac binding config\n"
		  "sta ip mac binding enable|disable\n"
	 )
{	
	int ret;
	unsigned int radioid = 0;
	unsigned int i = 0;
    int policy = 0;
	int timer = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;

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

	if(vty->node == RADIO_NODE){
		index = 0;			
		radioid = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = (int)vty->index; 	
		localid = vty->local;
        slot_id = vty->slotindex;
		radioid = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radioid = (int)vty->index_sub;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ret = dcli_radio_timer_able(localid,index,policy,timer,radioid,dcli_dbus_connection);
	if(ret == 0)
		vty_out(vty,"set radio %s %s successfully\n",argv[0],argv[1]);
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	return CMD_SUCCESS; 

}
//weichao add 
DEFUN(set_bss_multi_user_optimize_cmd_func,
	set_bss_multi_user_optimize_cmd,
	"set bss wlan WLANID multi_user switch (enable|disable)",
	"Set command\n"
	"set bss\n"
	"based bss\n"
	"wlan:<1~128>\n"
	"multi_user\n"
	"switch\n"
	"default disable"
)
{
	int ret = 0 ; 
	DBusMessage *query,*reply;
	DBusError err;
	DBusMessageIter iter;
	unsigned int radioid = 0 ; 
	unsigned char type = 0 ; 
	unsigned int wlan_id = 0; 
	unsigned char wlanid = 0 ;
	
	ret = parse_int_ID((char *)argv[0],&wlan_id);    
	if(ret != WID_DBUS_SUCCESS)
	{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}
	wlanid = (char)wlan_id;
	if((wlanid < 1 )||(wlanid > WLAN_NUM-1))
	{
		vty_out(vty,"<error> wlanid should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	if(!strcmp(argv[1],"enable"))
	{
		type = 1; 
	}
	else if(!strcmp(argv[1],"disable"))
	{
		type = 0 ; 
	}
	dbus_error_init(&err);
	
	int index = 0;
	int localid = 1;
   	 int slot_id = HostSlotId;
	if(vty->node == RADIO_NODE){
		index = 0;			
		radioid = (int)vty->index;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = vty->index; 
		localid = vty->local;
        		slot_id = vty->slotindex;
		radioid = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
	         index = vty->index;
	         localid = vty->local;
	         slot_id = vty->slotindex;
		radioid = (int)vty->index_sub;
    }
    	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_MOLTI_USER_OPTIMIZE_SWITH);
	dbus_message_append_args(query,
					DBUS_TYPE_BYTE,&wlanid,
					DBUS_TYPE_UINT32,&radioid,
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
		vty_out(vty,"set bss wlan %d multi_user switch %s successfully!\n",wlanid,argv[1]);
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

DEFUN( set_wsm_sta_info_reportswitch_cmd_func,
	  set_wsm_sta_info_reportswitch_cmd,
	  "set wlanid WLANID wsm sta info reportswitch ( enable |disable)",
	  "set wsm sta info\n"
	  "set special wlan station information report-switch or report-interval\n"
	  "wlanid 1-255"
	  "set special wlan station information report-switch or report-interval\n"
	  "set special wlan station information report-switch or report-interval\n"
	  "set special wlan station information report-switch or report-interval\n"
	  "report switch enable or disable\n"
	  "set special wlan station information report-switch\n"
	  "set special wlan station information report-switch enable\n"
	  "set special wlan station information report-switch disable\n"
	 )
{
	DBusMessage *query = NULL, *reply = NULL;	
	DBusMessageIter	 iter;
	DBusError err;	
	unsigned int radio_id; 
	unsigned char wlanid = 0; 
	char policy = 0;
	int ret = 0;
	ret = parse_char_ID((char *)argv[0],&wlanid);
	if((wlanid <= 0) /*||(wlanid > 255)*/)
	{
		vty_out(vty,"input wlanid should be 1-255\n");
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
	int index = 0;
	int localid = 1;
	int slot_id = HostSlotId;
	switch(vty->node)
	{
		case RADIO_NODE:
			index = 0;			
			radio_id = (int)vty->index;
			break;
		case HANSI_RADIO_NODE:
			index = (int)vty->index; 
			localid = (int)vty->local;
			slot_id = (int)vty->slotindex;
			radio_id = (unsigned int)vty->index_sub;
			break;
		case LOCAL_HANSI_RADIO_NODE:
			index = (int)vty->index;
			localid = (int)vty->local;
			slot_id = (int)vty->slotindex;
			radio_id = (unsigned int)vty->index_sub;
			break;
		default:
			return CMD_SUCCESS;
			break;
	}
    	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_WSM_STA_INFO_REPORTSWITCH);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&radio_id,
							DBUS_TYPE_BYTE,&wlanid,
							DBUS_TYPE_BYTE,&policy,
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
	switch (ret)
	{
		case WTP_ID_NOT_EXIST:
			vty_out(vty,"<error> wtp isn't existed\n");
			break;
		case RADIO_ID_NOT_EXIST:
			vty_out(vty,"<error> radio isn't existed\n");
			break;
		case RADIO_NO_BINDING_WLAN:
			vty_out(vty,"<error> radio doesn't bing wlan \n");
			break;
		case WID_DBUS_SUCCESS:
			vty_out(vty,"set wlanid %s wsm sta info reportswitch %s \n",argv[0],argv[1]);
			break;
		case WID_WANT_TO_DELETE_WLAN:		/* Huangleilei add for ASXXZFI-1622 */
			vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
			break;
		default:
			vty_out(vty,"<error> other error: %d\n", ret);
			break;
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS; 
}
DEFUN( set_wsm_sta_info_reportinterval_cmd_func,
	  set_wsm_sta_info_reportinterval_cmd,
	  "set wlanid WLANID wsm sta info reportinterval INTERVAL",
	  "set wsm sta info\n"
	  "set special wlan station information report-switch or report-interval\n"
	  "wlanid 1-255\n"
	  "set special wlan station information report-switch or report-interval\n"
	  "set special wlan station information report-switch or report-interval\n"
	  "set special wlan station information report-switch or report-interval\n"
	  "set special wlan station information report-interval\n"
	  "interval 30-65535\n"
	 )
{
	DBusMessage *query = NULL, *reply = NULL;	
	DBusMessageIter	 iter;
	DBusError err;	
	unsigned int radio_id = 0; 
	unsigned char wlanid = 0; 
	unsigned short interval = 0;
	int ret = 0;
	ret = parse_char_ID((char *)argv[0],&wlanid);
	if((wlanid <= 0) /*||(wlanid > 255)*/)
	{
		vty_out(vty,"input wlanid should be 1-255\n");
		return CMD_SUCCESS;
	}
	ret = parse_short_ID((char *)argv[1],&interval);
	if((interval < 30)/*||(interval > 65535)*/)
	{
		vty_out(vty,"input interval should be 30-65535\n");
		return CMD_SUCCESS;
	}
	int index = 0;
	int localid = 1;
	int slot_id = HostSlotId;
	switch(vty->node)
	{
		case RADIO_NODE:
			index = 0;			
			radio_id = (unsigned int)vty->index;
			break;
		case HANSI_RADIO_NODE:
			index = (int)vty->index; 
			localid = (int)vty->local;
			slot_id = (int)vty->slotindex;
			radio_id = (unsigned int)vty->index_sub;
			break;
		case LOCAL_HANSI_RADIO_NODE:
			index = (int)vty->index;
			localid = (int)vty->local;
			slot_id = (int)vty->slotindex;
			radio_id = (unsigned int)vty->index_sub;
			break;
		default:
			return CMD_SUCCESS;
			break;
	}
    	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_WSM_STA_INFO_REPORTINTERVAL);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&radio_id,
							DBUS_TYPE_BYTE,&wlanid,
							DBUS_TYPE_UINT16,&interval,
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
	switch (ret)
	{
		case WID_DBUS_SUCCESS:
			vty_out(vty,"set wlanid %s wsm sta info reportinterval %s \n",argv[0],argv[1]);
			break;
		case WTP_ID_NOT_EXIST:
			vty_out(vty,"<error> wtp isn't existed\n");
			break;
		case RADIO_ID_NOT_EXIST:
			vty_out(vty,"<error> radio isn't existed\n");
			break;
		case RADIO_NO_BINDING_WLAN:
			vty_out(vty,"<error> radio doesn't bing wlan \n");
			break;
		case WID_WANT_TO_DELETE_WLAN:		/* Huangleilei add for ASXXZFI-1622 */
			vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
			break;
		default :
			vty_out(vty,"<error> other error: %d\n", ret);
			break;
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS; 	
}

////////////////////////////////////////////////////////////////////////////////////
void dcli_radio_init(void) {
#if 0
	install_node(&radio_node,NULL);
	install_default(RADIO_NODE);	

	/************************************************AP_GROUP_NODE**************************************************/
	install_element(AP_GROUP_RADIO_NODE,&set_radio_txpowerstep_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_tx_chainmask_v2_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_11n_channel_offset_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_tx_chainmask_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_11n_puren_mixed_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_11n_ampdu_subframe_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_11n_ampdu_limit_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_11n_ampdu_able_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_wtp_sta_static_arp_enable_cmd);
	install_element(AP_GROUP_RADIO_NODE,&wds_aes_key_cmd); 
	install_element(AP_GROUP_RADIO_NODE,&wds_wep_key_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_congestion_avoidance_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_keep_alive_idle_time_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_keep_alive_period_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_intra_vap_forwarding_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_inter_vap_forwarding_cmd);
	install_element(AP_GROUP_RADIO_NODE,&wds_encryption_type_cmd);
	install_element(AP_GROUP_RADIO_NODE,&wds_remote_brmac_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_wds_bridge_distance_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_netgear_supper_g_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_sector_power_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_sector_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_cmmode_cmd); 
	install_element(AP_GROUP_RADIO_NODE,&set_radio_mcs_cmd);  
	install_element(AP_GROUP_RADIO_NODE,&set_radio_guard_interval_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_sta_ip_mac_binding_cmd);  
	install_element(AP_GROUP_RADIO_NODE,&set_sta_dhcp_before_authorized_cmd); 
	install_element(AP_GROUP_RADIO_NODE,&radio_apply_wlan_clean_vlan_cmd);
	install_element(AP_GROUP_RADIO_NODE,&radio_bss_taffic_limit_average_send_value_cmd);
	install_element(AP_GROUP_RADIO_NODE,&radio_bss_taffic_limit_send_value_cmd);
	install_element(AP_GROUP_RADIO_NODE,&radio_bss_taffic_limit_average_value_cmd);
	install_element(AP_GROUP_RADIO_NODE,&radio_bss_taffic_limit_value_cmd);
	install_element(AP_GROUP_RADIO_NODE,&radio_bss_taffic_limit_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_ap_radio_txantenna_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_ap_radio_diversity_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_ap_radio_auto_channel_cont_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_ap_radio_auto_channel_cmd);
	install_element(AP_GROUP_RADIO_NODE,&radio_wlan_wds_bssid_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_11n_cwmmode_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_l2_isolation_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_max_throughout_cmd);
	install_element(AP_GROUP_RADIO_NODE,&radio_apply_wlan_base_vlan_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_default_config_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_disable_wlan_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_enable_wlan_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_delete_wlan_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_bss_max_throughput_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_delete_qos_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_apply_qos_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_apply_wlan_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_bss_l3_policy_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_shortretry_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_longretry_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_preamble_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_wds_service_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_service_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_rtsthreshold_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_dtim_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_fragmentation_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_beaconinterval_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_txpowerof_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_mode_cmd);
	install_element(AP_GROUP_RADIO_NODE,&radio_bss_taffic_limit_cancel_sta_send_value_cmd); 
	install_element(AP_GROUP_RADIO_NODE,&radio_bss_taffic_limit_cancel_sta_value_cmd); 
	install_element(AP_GROUP_RADIO_NODE,&set_radio_txpower_cmd);
	install_element(AP_GROUP_RADIO_NODE,&radio_bss_taffic_limit_sta_send_value_cmd); 
	install_element(AP_GROUP_RADIO_NODE,&set_radio_bss_max_num_cmd); 
	install_element(AP_GROUP_RADIO_NODE,&set_radio_channel_cmd); 
	install_element(AP_GROUP_RADIO_NODE,&set_radio_max_rate_cmd);
	install_element(AP_GROUP_RADIO_NODE,&set_radio_ratelist_cmd);
	/************************************************AP_GROUP_NODE_END**************************************************/
#endif
	/************************************************HANSI_AP_GROUP_NODE**************************************************/
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_txpowerstep_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_tx_chainmask_v2_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_11n_channel_offset_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_tx_chainmask_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_11n_puren_mixed_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_11n_ampdu_subframe_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_11n_ampdu_limit_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_11n_ampdu_able_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_wtp_sta_static_arp_enable_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&wds_aes_key_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&wds_wep_key_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_congestion_avoidance_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_keep_alive_idle_time_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_keep_alive_period_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_intra_vap_forwarding_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_inter_vap_forwarding_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&wds_encryption_type_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&wds_remote_brmac_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_wds_bridge_distance_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_netgear_supper_g_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_sector_power_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_sector_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_cmmode_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_mcs_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_guard_interval_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_sta_ip_mac_binding_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_sta_dhcp_before_authorized_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&radio_apply_wlan_clean_vlan_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&radio_bss_taffic_limit_average_send_value_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&radio_bss_taffic_limit_send_value_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&radio_bss_taffic_limit_average_value_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&radio_bss_taffic_limit_value_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&radio_bss_taffic_limit_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_ap_radio_txantenna_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_ap_radio_diversity_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_ap_radio_auto_channel_cont_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_ap_radio_auto_channel_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&radio_wlan_wds_bssid_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_11n_cwmmode_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_l2_isolation_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_max_throughout_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&radio_apply_wlan_base_vlan_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_default_config_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_disable_wlan_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_enable_wlan_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_delete_wlan_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_bss_max_throughput_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_delete_qos_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_apply_qos_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_apply_wlan_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_bss_l3_policy_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_shortretry_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_longretry_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_preamble_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_wds_service_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_service_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_rtsthreshold_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_dtim_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_fragmentation_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_beaconinterval_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_txpowerof_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_mode_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&radio_bss_taffic_limit_cancel_sta_send_value_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&radio_bss_taffic_limit_cancel_sta_value_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_txpower_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&radio_bss_taffic_limit_sta_send_value_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_bss_max_num_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_channel_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_max_rate_cmd);
	install_element(HANSI_AP_GROUP_RADIO_NODE,&set_radio_ratelist_cmd);
	/************************************************HANSI_AP_GROUP_NODE_END**************************************************/

	/************************************************LOCAL_HANSI_AP_GROUP_NODE**************************************************/
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_txpowerstep_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_tx_chainmask_v2_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_11n_channel_offset_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_tx_chainmask_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_11n_puren_mixed_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_11n_ampdu_subframe_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_11n_ampdu_limit_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_11n_ampdu_able_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_wtp_sta_static_arp_enable_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&wds_aes_key_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&wds_wep_key_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_congestion_avoidance_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_keep_alive_idle_time_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_keep_alive_period_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_intra_vap_forwarding_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_inter_vap_forwarding_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&wds_encryption_type_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&wds_remote_brmac_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_wds_bridge_distance_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_netgear_supper_g_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_sector_power_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_sector_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_cmmode_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_mcs_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_guard_interval_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_sta_ip_mac_binding_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_sta_dhcp_before_authorized_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&radio_apply_wlan_clean_vlan_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&radio_bss_taffic_limit_average_send_value_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&radio_bss_taffic_limit_send_value_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&radio_bss_taffic_limit_average_value_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&radio_bss_taffic_limit_value_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&radio_bss_taffic_limit_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_ap_radio_txantenna_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_ap_radio_diversity_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_ap_radio_auto_channel_cont_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_ap_radio_auto_channel_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&radio_wlan_wds_bssid_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_11n_cwmmode_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_l2_isolation_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_max_throughout_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&radio_apply_wlan_base_vlan_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_default_config_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_disable_wlan_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_enable_wlan_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_delete_wlan_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_bss_max_throughput_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_delete_qos_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_apply_qos_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_apply_wlan_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_bss_l3_policy_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_shortretry_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_longretry_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_preamble_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_wds_service_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_service_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_rtsthreshold_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_dtim_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_fragmentation_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_beaconinterval_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_txpowerof_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_mode_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&radio_bss_taffic_limit_cancel_sta_send_value_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&radio_bss_taffic_limit_cancel_sta_value_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_txpower_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&radio_bss_taffic_limit_sta_send_value_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_bss_max_num_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_channel_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_max_rate_cmd);
	install_element(LOCAL_HANSI_AP_GROUP_RADIO_NODE,&set_radio_ratelist_cmd);
	/************************************************LOCAL_HANSI_AP_GROUP_NODE_END**************************************************/
	install_element(VIEW_NODE,&show_radio_cmd);	
	install_element(VIEW_NODE,&show_radio_list_cmd);
	install_element(VIEW_NODE,&show_radio_qos_cmd);
#if 0	
	/*------------------------------VIEW_NODE-----------------------------*/
	install_element(VIEW_NODE,&show_radio_list_cmd);												/*a1*/
	install_element(VIEW_NODE,&show_radio_cmd);														/*a2*/
	install_element(VIEW_NODE,&show_radio_qos_cmd);													/*a3*/
	
	/*------------------------------ENABLE_NODE-----------------------------*/
	install_element(ENABLE_NODE,&show_radio_list_cmd);												/*a1*/
	install_element(ENABLE_NODE,&show_radio_cmd);													/*a2*/
	install_element(ENABLE_NODE,&show_radio_qos_cmd);												/*a3*/
	
	/*------------------------------CONFIG_NODE-----------------------------*/
	install_element(CONFIG_NODE,&show_radio_list_cmd);												/*a1*/
	install_element(CONFIG_NODE,&show_radio_cmd);													/*a2*/
	install_element(CONFIG_NODE,&show_radio_qos_cmd);												/*a3*/
	
	install_element(CONFIG_NODE,&config_radio_cmd);
	install_element(CONFIG_NODE,&set_wtp_list_sta_static_arp_enable_cmd);
	install_element(CONFIG_NODE,&set_sta_mac_vlanid_cmd); /*ht add 091028*/
	
	/*------------------------------RADIO_NODE-----------------------------*/
	install_element(RADIO_NODE,&set_sta_dhcp_before_authorized_cmd); /*ht add 091028*/
	install_element(RADIO_NODE,&set_sta_ip_mac_binding_cmd); /*ht add 091028*/
	install_element(RADIO_NODE,&set_radio_channel_cmd);
	install_element(RADIO_NODE,&set_radio_bss_max_num_cmd);/*  xm add  08/12/01*/
	install_element(RADIO_NODE,&set_radio_txpower_cmd);	
	install_element(RADIO_NODE,&set_radio_txpowerof_cmd);	/*  wuwl add  0928*/
/*	install_element(RADIO_NODE,&set_radio_rate_cmd);	*/
	install_element(RADIO_NODE,&set_radio_mode_cmd);
	install_element(RADIO_NODE,&set_radio_ratelist_cmd);
	install_element(RADIO_NODE,&set_radio_max_rate_cmd);
	
	install_element(RADIO_NODE,&set_radio_beaconinterval_cmd);	
	install_element(RADIO_NODE,&set_radio_fragmentation_cmd);	
	install_element(RADIO_NODE,&set_radio_dtim_cmd);	
	install_element(RADIO_NODE,&set_radio_rtsthreshold_cmd);	
	install_element(RADIO_NODE,&set_radio_service_cmd);	
	install_element(RADIO_NODE,&set_wds_service_cmd);	
	install_element(RADIO_NODE,&set_radio_preamble_cmd);
	install_element(RADIO_NODE,&set_radio_shortretry_cmd);
	install_element(RADIO_NODE,&set_radio_longretry_cmd);
	install_element(RADIO_NODE,&set_radio_apply_qos_cmd);
	install_element(RADIO_NODE,&set_radio_delete_qos_cmd);
	
	install_element(RADIO_NODE,&set_radio_bss_l3_policy_cmd);
	install_element(RADIO_NODE,&set_radio_bss_max_throughput_cmd);
	install_element(RADIO_NODE,&show_radio_bss_max_throughput_cmd);
	install_element(RADIO_NODE,&set_radio_apply_wlan_cmd);
	install_element(RADIO_NODE,&set_radio_delete_wlan_cmd);
	install_element(RADIO_NODE,&set_radio_enable_wlan_cmd);
	install_element(RADIO_NODE,&set_radio_disable_wlan_cmd);
	install_element(RADIO_NODE,&set_radio_default_config_cmd);
	
	install_element(RADIO_NODE,&show_radio_channel_change_cmd);
	install_element(RADIO_NODE,&radio_apply_wlan_base_vlan_cmd);
	install_element(RADIO_NODE,&radio_apply_wlan_clean_vlan_cmd);
	install_element(RADIO_NODE,&radio_apply_wlan_base_nas_port_id_cmd);		//mahz add 2011.5.30
	install_element(RADIO_NODE,&radio_apply_wlan_clean_nas_port_id_cmd);
	/*install_element(RADIO_NODE,&set_radio_bss_l3_interface_br_cmd);*/
	install_element(RADIO_NODE,&set_radio_max_throughout_cmd);
	install_element(RADIO_NODE,&show_radio_bss_cmd);
	install_element(RADIO_NODE,&set_radio_l2_isolation_cmd);
	install_element(RADIO_NODE,&set_radio_11n_cwmmode_cmd);
	install_element(RADIO_NODE,&radio_wlan_wds_bssid_cmd);
	install_element(RADIO_NODE,&show_wlan_wds_bssid_list_cmd);
	install_element(RADIO_NODE,&set_ap_radio_auto_channel_cmd);
	install_element(RADIO_NODE,&set_ap_radio_auto_channel_cont_cmd);
	install_element(RADIO_NODE,&set_ap_radio_diversity_cmd);
	install_element(RADIO_NODE,&set_ap_radio_txantenna_cmd);
	install_element(RADIO_NODE,&radio_bss_taffic_limit_cmd);
	install_element(RADIO_NODE,&radio_bss_taffic_limit_value_cmd);
	install_element(RADIO_NODE,&radio_bss_taffic_limit_average_value_cmd);
	install_element(RADIO_NODE,&radio_bss_taffic_limit_sta_value_cmd);
	install_element(RADIO_NODE,&radio_bss_taffic_limit_cancel_sta_value_cmd);
	install_element(RADIO_NODE,&radio_bss_taffic_limit_send_value_cmd);
	install_element(RADIO_NODE,&radio_bss_taffic_limit_average_send_value_cmd);
	install_element(RADIO_NODE,&radio_bss_taffic_limit_sta_send_value_cmd);
	install_element(RADIO_NODE,&radio_bss_taffic_limit_cancel_sta_send_value_cmd);
	install_element(RADIO_NODE,&set_radio_guard_interval_cmd);
	install_element(RADIO_NODE,&set_radio_mcs_cmd);
	install_element(RADIO_NODE,&set_radio_cmmode_cmd);

	install_element(RADIO_NODE,&set_radio_sector_cmd);	
	//install_element(RADIO_NODE,&set_tx_chainmask_cmd);
	install_element(RADIO_NODE,&set_tx_chainmask_v2_cmd);
	install_element(RADIO_NODE,&set_radio_sector_power_cmd);
	install_element(RADIO_NODE,&set_radio_netgear_supper_g_cmd);
	install_element(RADIO_NODE,&set_wds_bridge_distance_cmd);
	install_element(RADIO_NODE,&wds_remote_brmac_cmd);
	install_element(RADIO_NODE,&wds_encryption_type_cmd);
	install_element(RADIO_NODE,&wds_wep_key_cmd);
	install_element(RADIO_NODE,&wds_aes_key_cmd);
	install_element(RADIO_NODE,&set_radio_inter_vap_forwarding_cmd);	
	install_element(RADIO_NODE,&set_radio_intra_vap_forwarding_cmd);
	install_element(RADIO_NODE,&set_radio_keep_alive_period_cmd);
	install_element(RADIO_NODE,&set_radio_keep_alive_idle_time_cmd);
	install_element(RADIO_NODE,&set_radio_congestion_avoidance_cmd);
	install_element(RADIO_NODE,&set_wtp_sta_static_arp_enable_cmd);
	install_element(RADIO_NODE,&set_radio_11n_ampdu_able_cmd);
	install_element(RADIO_NODE,&set_radio_11n_ampdu_limit_cmd);
	install_element(RADIO_NODE,&set_radio_11n_ampdu_subframe_cmd);//zhangshu add for subframe , 2010-10-09
	install_element(RADIO_NODE,&set_radio_11n_puren_mixed_cmd);
	install_element(RADIO_NODE,&set_radio_11n_channel_offset_cmd);
	install_element(RADIO_NODE,&set_radio_txpowerstep_cmd);/*zhaoruijia,20100910,add txpower step,start*/
	install_element(RADIO_NODE,&radio_rx_data_dead_time_cmd);
	install_element(RADIO_NODE,&show_radio_receive_data_dead_time_cmd);

	install_element(RADIO_NODE,&set_radio_servive_timer_func_cmd);
	install_element(RADIO_NODE,&set_radio_timer_able_cmd);	
	install_element(RADIO_NODE, &set_wsm_sta_info_reportswitch_cmd);
	install_element(RADIO_NODE, &set_wsm_sta_info_reportinterval_cmd);

#endif





/********************************************************************************/
#if 0 /**wangchao moved to dcli_wireless_main.c**/
	install_node(&hansi_radio_node,NULL,"HANSI_RADIO_NODE");
	install_default(HANSI_RADIO_NODE);
#endif	
	/*------------------------------HANSI_NODE-----------------------------*/
	install_element(HANSI_NODE,&show_radio_cmd);	
	install_element(HANSI_NODE,&show_radio_list_cmd);
	install_element(HANSI_NODE,&show_radio_qos_cmd);
	install_element(HANSI_NODE,&config_radio_cmd);			
	install_element(HANSI_NODE,&set_sta_mac_vlanid_cmd); /*ht add 091028*/
	install_element(HANSI_NODE,&set_wtp_list_sta_static_arp_enable_cmd);
	
	/*------------------------------HANSI_RADIO_NODE-----------------------------*/
	install_element(HANSI_RADIO_NODE,&set_radio_channel_cmd);
	install_element(HANSI_RADIO_NODE,&set_radio_bss_max_num_cmd);/*  xm add  08/12/01*/
	install_element(HANSI_RADIO_NODE,&set_radio_txpower_cmd); 
	install_element(HANSI_RADIO_NODE,&set_radio_txpowerof_cmd);	/*	wuwl add  0928*/
/*	install_element(RADIO_NODE,&set_radio_rate_cmd);	*/
	install_element(HANSI_RADIO_NODE,&set_radio_mode_cmd);
    install_element(HANSI_RADIO_NODE,&set_radio_management_frame_rate_cmd);
	install_element(HANSI_RADIO_NODE,&radio_clear_rate_for_wlan_cmd);	  
	install_element(HANSI_RADIO_NODE,&set_radio_ratelist_cmd);
	install_element(HANSI_RADIO_NODE,&set_radio_max_rate_cmd);
	install_element(HANSI_RADIO_NODE,&set_radio_beaconinterval_cmd);	
	install_element(HANSI_RADIO_NODE,&set_radio_fragmentation_cmd);	
	install_element(HANSI_RADIO_NODE,&set_radio_dtim_cmd);	
	install_element(HANSI_RADIO_NODE,&set_radio_rtsthreshold_cmd);	
	install_element(HANSI_RADIO_NODE,&set_radio_service_cmd); 
	install_element(HANSI_RADIO_NODE,&set_wds_service_cmd);	
	install_element(HANSI_RADIO_NODE,&set_radio_preamble_cmd);
	install_element(HANSI_RADIO_NODE,&set_radio_shortretry_cmd);
	install_element(HANSI_RADIO_NODE,&set_radio_longretry_cmd);
	install_element(HANSI_RADIO_NODE,&set_radio_apply_qos_cmd);
	install_element(HANSI_RADIO_NODE,&set_radio_delete_qos_cmd);
	
	install_element(HANSI_RADIO_NODE,&set_radio_bss_l3_policy_cmd);
	install_element(HANSI_RADIO_NODE,&set_radio_bss_max_throughput_cmd);
	install_element(HANSI_RADIO_NODE,&show_radio_bss_max_throughput_cmd);
	install_element(HANSI_RADIO_NODE,&set_radio_apply_wlan_cmd);
	install_element(HANSI_RADIO_NODE,&set_radio_apply_wlan_base_essid_cmd);
	install_element(HANSI_RADIO_NODE,&set_radio_delete_wlan_cmd);
	install_element(HANSI_RADIO_NODE,&set_radio_delete_wlan_base_essid_cmd);
	install_element(HANSI_RADIO_NODE,&set_radio_enable_wlan_cmd);
	install_element(HANSI_RADIO_NODE,&set_radio_disable_wlan_cmd);
	install_element(HANSI_RADIO_NODE,&set_radio_default_config_cmd);
	
	install_element(HANSI_RADIO_NODE,&show_radio_channel_change_cmd);
	install_element(HANSI_RADIO_NODE,&set_radio_cpe_channel_apply_wlan_base_vlan_cmd);
	install_element(HANSI_RADIO_NODE,&set_radio_cpe_channel_apply_wlan_clean_vlan_cmd);
	install_element(HANSI_RADIO_NODE,&set_radio_cpe_channel_apply_wlan_clean_vlan_id_cmd);
	install_element(HANSI_RADIO_NODE,&radio_apply_wlan_base_vlan_cmd);
	install_element(HANSI_RADIO_NODE,&radio_apply_wlan_clean_vlan_cmd);
	install_element(HANSI_RADIO_NODE,&radio_apply_wlan_base_nas_port_id_cmd);		//mahz add 2011.5.30
	install_element(HANSI_RADIO_NODE,&radio_apply_wlan_clean_nas_port_id_cmd);
	/*install_element(RADIO_NODE,&set_radio_bss_l3_interface_br_cmd);*/
	install_element(HANSI_RADIO_NODE,&set_radio_max_throughout_cmd);
	install_element(HANSI_RADIO_NODE,&show_radio_bss_cmd);
	install_element(HANSI_RADIO_NODE,&set_radio_l2_isolation_cmd);
	install_element(HANSI_RADIO_NODE,&set_radio_11n_cwmmode_cmd);
	install_element(HANSI_RADIO_NODE,&radio_wlan_wds_bssid_cmd);
	install_element(HANSI_RADIO_NODE,&show_wlan_wds_bssid_list_cmd);
	install_element(HANSI_RADIO_NODE,&set_ap_radio_auto_channel_cmd);
	install_element(HANSI_RADIO_NODE,&set_ap_radio_auto_channel_cont_cmd);
	install_element(HANSI_RADIO_NODE,&set_ap_radio_diversity_cmd);
	install_element(HANSI_RADIO_NODE,&set_ap_radio_txantenna_cmd);
	install_element(HANSI_RADIO_NODE,&radio_bss_taffic_limit_cmd);
	install_element(HANSI_RADIO_NODE,&radio_bss_taffic_limit_value_cmd);
	install_element(HANSI_RADIO_NODE,&radio_bss_taffic_limit_average_value_cmd);
	install_element(HANSI_RADIO_NODE,&radio_bss_taffic_limit_cancel_average_value_cmd);//fengwenchao add 20130416 for AXSSZFI-1374
	install_element(HANSI_RADIO_NODE,&radio_bss_taffic_limit_cancel_average_send_value_cmd);//fengwenchao add 20130416 for AXSSZFI-1374
	install_element(HANSI_RADIO_NODE,&radio_bss_taffic_limit_sta_value_cmd);
	install_element(HANSI_RADIO_NODE,&radio_bss_taffic_limit_cancel_sta_value_cmd);
	install_element(HANSI_RADIO_NODE,&radio_bss_taffic_limit_send_value_cmd);
	install_element(HANSI_RADIO_NODE,&radio_bss_taffic_limit_average_send_value_cmd);
	install_element(HANSI_RADIO_NODE,&radio_bss_taffic_limit_sta_send_value_cmd);
	install_element(HANSI_RADIO_NODE,&radio_bss_taffic_limit_cancel_sta_send_value_cmd);
	install_element(HANSI_RADIO_NODE,&set_sta_dhcp_before_authorized_cmd); /*ht add 091028*/
	install_element(HANSI_RADIO_NODE,&set_sta_ip_mac_binding_cmd); /*ht add 091028*/
	install_element(HANSI_RADIO_NODE,&set_radio_guard_interval_cmd);
	install_element(HANSI_RADIO_NODE,&set_radio_acktimeout_distance_cmd);/*wcl add for RDIR-33*/
	install_element(HANSI_RADIO_NODE,&set_radio_mcs_cmd);
	install_element(HANSI_RADIO_NODE,&set_radio_cmmode_cmd);
	install_element(HANSI_RADIO_NODE,&set_radio_sector_cmd);		
	//install_element(HANSI_RADIO_NODE,&set_tx_chainmask_cmd);
	install_element(HANSI_RADIO_NODE,&set_tx_chainmask_v2_cmd);
	install_element(HANSI_RADIO_NODE,&set_radio_sector_power_cmd);
	install_element(HANSI_RADIO_NODE,&set_radio_netgear_supper_g_cmd);
	install_element(HANSI_RADIO_NODE,&set_wds_bridge_distance_cmd);
	install_element(HANSI_RADIO_NODE,&wds_remote_brmac_cmd);
	install_element(HANSI_RADIO_NODE,&wds_encryption_type_cmd);
	install_element(HANSI_RADIO_NODE,&wds_wep_key_cmd);
	install_element(HANSI_RADIO_NODE,&wds_aes_key_cmd);
	install_element(HANSI_RADIO_NODE,&set_radio_inter_vap_forwarding_cmd);		
	install_element(HANSI_RADIO_NODE,&set_radio_intra_vap_forwarding_cmd);
	install_element(HANSI_RADIO_NODE,&set_radio_keep_alive_period_cmd);	
	install_element(HANSI_RADIO_NODE,&set_radio_keep_alive_idle_time_cmd);	
	install_element(HANSI_RADIO_NODE,&set_radio_congestion_avoidance_cmd);
	install_element(HANSI_RADIO_NODE,&set_wtp_sta_static_arp_enable_cmd);
	install_element(HANSI_RADIO_NODE,&set_radio_11n_ampdu_able_cmd);
	install_element(HANSI_RADIO_NODE,&set_radio_11n_ampdu_limit_cmd);
	install_element(HANSI_RADIO_NODE,&set_radio_11n_ampdu_subframe_cmd);//zhangshu add for subframe , 2010-10-09
	install_element(HANSI_RADIO_NODE,&set_radio_11n_puren_mixed_cmd);
	install_element(HANSI_RADIO_NODE,&set_radio_11n_channel_offset_cmd);
	install_element(HANSI_RADIO_NODE,&set_radio_txpowerstep_cmd);/*zhaoruijia,20100910,add txpower step,start*/
	install_element(HANSI_RADIO_NODE,&radio_rx_data_dead_time_cmd);
	install_element(HANSI_RADIO_NODE,&show_radio_receive_data_dead_time_cmd);
	install_element(HANSI_RADIO_NODE,&set_radio_wlan_limit_rssi_access_sta_cmd); //fengwenchao add 20120222 for RDIR-25
	install_element(HANSI_RADIO_NODE,&set_radio_servive_timer_func_cmd);
	install_element(HANSI_RADIO_NODE,&set_radio_timer_able_cmd);	
	
	install_element(HANSI_RADIO_NODE,&radio_apply_wlan_base_hotspot_cmd);	
	install_element(HANSI_RADIO_NODE,&radio_apply_wlan_clean_hotspot_cmd);	
	install_element(HANSI_RADIO_NODE,&set_bss_multi_user_optimize_cmd);	
	install_element(HANSI_RADIO_NODE, &set_wsm_sta_info_reportswitch_cmd);
	install_element(HANSI_RADIO_NODE, &set_wsm_sta_info_reportinterval_cmd);
    /********************************************************************************/
/****wangchao moved to dcli_wireless.c****/
#if 0
	install_node(&local_hansi_radio_node,NULL,"LOCAL_HANSI_RADIO_NODE");
	install_default(LOCAL_HANSI_RADIO_NODE);
#endif	
	/*------------------------------HANSI_NODE-----------------------------*/
	install_element(LOCAL_HANSI_NODE,&show_radio_cmd);	
	install_element(LOCAL_HANSI_NODE,&show_radio_list_cmd);
	install_element(LOCAL_HANSI_NODE,&show_radio_qos_cmd);
	install_element(LOCAL_HANSI_NODE,&config_radio_cmd);			
	install_element(LOCAL_HANSI_NODE,&set_sta_mac_vlanid_cmd); /*ht add 091028*/
	install_element(LOCAL_HANSI_NODE,&set_wtp_list_sta_static_arp_enable_cmd);
	
	/*------------------------------HANSI_RADIO_NODE-----------------------------*/
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_channel_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_bss_max_num_cmd);/*  xm add  08/12/01*/
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_txpower_cmd); 
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_txpowerof_cmd);	/*	wuwl add  0928*/
/*	install_element(RADIO_NODE,&set_radio_rate_cmd);	*/
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_mode_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_ratelist_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_max_rate_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_beaconinterval_cmd);	
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_fragmentation_cmd);	
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_dtim_cmd);	
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_rtsthreshold_cmd);	
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_service_cmd); 
	install_element(LOCAL_HANSI_RADIO_NODE,&set_wds_service_cmd);	
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_preamble_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_shortretry_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_longretry_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_apply_qos_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_delete_qos_cmd);
	
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_bss_l3_policy_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_bss_max_throughput_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&show_radio_bss_max_throughput_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_apply_wlan_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_delete_wlan_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_enable_wlan_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_disable_wlan_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_default_config_cmd);
	
	install_element(LOCAL_HANSI_RADIO_NODE,&show_radio_channel_change_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&radio_apply_wlan_base_vlan_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&radio_apply_wlan_clean_vlan_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&radio_apply_wlan_base_nas_port_id_cmd);		//mahz add 2011.5.30
	install_element(LOCAL_HANSI_RADIO_NODE,&radio_apply_wlan_clean_nas_port_id_cmd);
	/*install_element(RADIO_NODE,&set_radio_bss_l3_interface_br_cmd);*/
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_max_throughout_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&show_radio_bss_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_l2_isolation_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_11n_cwmmode_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&radio_wlan_wds_bssid_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&show_wlan_wds_bssid_list_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_ap_radio_auto_channel_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_ap_radio_auto_channel_cont_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_ap_radio_diversity_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_ap_radio_txantenna_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&radio_bss_taffic_limit_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&radio_bss_taffic_limit_value_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&radio_bss_taffic_limit_average_value_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&radio_bss_taffic_limit_cancel_average_value_cmd);//fengwenchao add 20130416 for AXSSZFI-1374
	install_element(LOCAL_HANSI_RADIO_NODE,&radio_bss_taffic_limit_cancel_average_send_value_cmd);//fengwenchao add 20130416 for AXSSZFI-1374
	install_element(LOCAL_HANSI_RADIO_NODE,&radio_bss_taffic_limit_sta_value_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&radio_bss_taffic_limit_cancel_sta_value_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&radio_bss_taffic_limit_send_value_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&radio_bss_taffic_limit_average_send_value_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&radio_bss_taffic_limit_sta_send_value_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&radio_bss_taffic_limit_cancel_sta_send_value_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_sta_dhcp_before_authorized_cmd); /*ht add 091028*/
	install_element(LOCAL_HANSI_RADIO_NODE,&set_sta_ip_mac_binding_cmd); /*ht add 091028*/
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_guard_interval_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_mcs_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_cmmode_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_sector_cmd);		
	//install_element(HANSI_RADIO_NODE,&set_tx_chainmask_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_tx_chainmask_v2_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_sector_power_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_netgear_supper_g_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_wds_bridge_distance_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&wds_remote_brmac_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&wds_encryption_type_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&wds_wep_key_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&wds_aes_key_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_inter_vap_forwarding_cmd);		
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_intra_vap_forwarding_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_keep_alive_period_cmd);	
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_keep_alive_idle_time_cmd);	
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_congestion_avoidance_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_wtp_sta_static_arp_enable_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_11n_ampdu_able_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_11n_ampdu_limit_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_11n_ampdu_subframe_cmd);//zhangshu add for subframe , 2010-10-09
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_11n_puren_mixed_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_11n_channel_offset_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_txpowerstep_cmd);/*zhaoruijia,20100910,add txpower step,start*/
	install_element(LOCAL_HANSI_RADIO_NODE,&radio_rx_data_dead_time_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&show_radio_receive_data_dead_time_cmd);

	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_servive_timer_func_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_timer_able_cmd);	
	install_element(LOCAL_HANSI_RADIO_NODE,&radio_apply_wlan_base_hotspot_cmd);	
	install_element(LOCAL_HANSI_RADIO_NODE,&radio_apply_wlan_clean_hotspot_cmd);	
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_wlan_limit_rssi_access_sta_cmd); //fengwenchao add 20120222 for RDIR-25
	install_element(LOCAL_HANSI_RADIO_NODE, &set_wsm_sta_info_reportswitch_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE, &set_wsm_sta_info_reportinterval_cmd);
	install_element(HANSI_WLAN_NODE,&interface_wlan_tunnel_mode_cmd);
	install_element(HANSI_NODE,&interface_wlan_tunnel_mode_cmd);
	install_element(HANSI_RADIO_NODE,&interface_radio_tunnel_mode_cmd);
	
	install_element(LOCAL_HANSI_WLAN_NODE,&interface_wlan_tunnel_mode_cmd);
	install_element(LOCAL_HANSI_NODE,&interface_wlan_tunnel_mode_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&interface_radio_tunnel_mode_cmd);
	install_element(HANSI_RADIO_NODE,&set_ap_uni_muti_bro_cast_isolation_set_cmd); 
	install_element(HANSI_RADIO_NODE,&set_ap_not_response_sta_probe_request_cmd);  
	install_element(HANSI_RADIO_NODE,&set_ap_muti_bro_cast_rate_set_cmd);  
	install_element(LOCAL_HANSI_RADIO_NODE,&set_ap_uni_muti_bro_cast_isolation_set_cmd); 
	install_element(LOCAL_HANSI_RADIO_NODE,&set_ap_not_response_sta_probe_request_cmd);  
	install_element(LOCAL_HANSI_RADIO_NODE,&set_ap_muti_bro_cast_rate_set_cmd);  
	install_element(LOCAL_HANSI_RADIO_NODE,&set_bss_multi_user_optimize_cmd);	
	install_element(LOCAL_HANSI_RADIO_NODE,&set_radio_acktimeout_distance_cmd);/*wcl add for RDIR-33*/
}

#endif
