#include <string.h>
#include <dbus/dbus.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include "iuh/Iuh.h"
#include "Iuh_DBus.h"
#include "Iuh_Thread.h"
#include "Iuh_log.h"
#include "Iuh_DBus_handler.h"
#include "dbus/iuh/IuhDBusDef.h"

#define IUH_SAVE_CFG_MEM (10*1024)

static DBusConnection * iuh_dbus_connection = NULL;

int IuhDBUS_MSGQ;
extern int asn_debug_switch;


/*****************************************************
** DISCRIPTION:
**          bing dynamic hnb login interface by ifname
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/
DBusMessage * iuh_dbus_dynamic_hnb_login_ifname(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	DBusError err;
	dbus_error_init(&err);
	int ret = IUH_DBUS_SUCCESS;
	char* ifname;
	unsigned char policy = 2;

	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&policy,
								DBUS_TYPE_STRING,&ifname,
								DBUS_TYPE_INVALID))){

		iuh_syslog_debug_debug(IUH_DEFAULT,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			iuh_syslog_debug_debug(IUH_DEFAULT,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
    
	if(g_auto_hnb_login.auto_hnb_switch == 1){
		ret = SWITCH_IS_DISABLE;//config required to disable the switch
	}
	else{
	    ret = Check_And_Bind_Interface_For_HNB(ifname, policy);
	}
	
    
	if(ret == 0){
		if(policy == 1){
		    ret = iuh_auto_hnb_login_insert_iflist(ifname);
		}
		else if(policy == 0){
		    if(g_auto_hnb_login.ifnum != 0){
			    ret = iuh_auto_hnb_login_remove_iflist(ifname);
			}
		}
	}
	
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);

	
	return reply;	

}


/*****************************************************
** DISCRIPTION:
**          set iuh log level
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/
DBusMessage *iuh_dbus_set_iuh_daemonlog_debug(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusError err;
	dbus_error_init(&err);
	char loglevel[20];	
	int ret = IUH_DBUS_SUCCESS;
	unsigned int daemonlogtype = 0;
	unsigned int daemonloglevel = 0;

	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&daemonlogtype,
								DBUS_TYPE_UINT32,&daemonloglevel,
								DBUS_TYPE_INVALID))){

		iuh_syslog_debug_debug(IUH_DEFAULT,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			iuh_syslog_debug_debug(IUH_DEFAULT,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);

	if(daemonloglevel == 1)	{
		gIuhLOGLEVEL |= daemonlogtype;
		strcpy(loglevel, "open");
	}	else if(daemonloglevel == 0)	{
		gIuhLOGLEVEL &= ~daemonlogtype;
		strcpy(loglevel, "close");
	}
	iuh_syslog_debug_debug(IUH_DEFAULT,"set Iuh daemonlog debug %d\n",gIuhLOGLEVEL);
	return reply;	

}


/*****************************************************
** DISCRIPTION:
**          set iuh rncid
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/
DBusMessage *iuh_dbus_set_rncid(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusError err;
	dbus_error_init(&err);
	int ret = IUH_DBUS_SUCCESS;
	unsigned short int rncid = 0;

	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT16,&rncid,
								DBUS_TYPE_INVALID))){

		iuh_syslog_debug_debug(IUH_DEFAULT,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			iuh_syslog_debug_debug(IUH_DEFAULT,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);

	gRncId = rncid;
	
	iuh_syslog_debug_debug(IUH_DEFAULT,"set gRncId to %d\n",gRncId);
	return reply;	

}


/*****************************************************
** DISCRIPTION:
**          set iuh asn log open/close
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/
DBusMessage *iuh_dbus_set_iuh_asn_debug(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusError err;
	dbus_error_init(&err);
	int ret = IUH_DBUS_SUCCESS;
	int debug_switch = 0;

	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&debug_switch,
								DBUS_TYPE_INVALID))){

		iuh_syslog_debug_debug(IUH_DEFAULT,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			iuh_syslog_debug_debug(IUH_DEFAULT,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);

	asn_debug_switch = debug_switch;
	
	iuh_syslog_debug_debug(IUH_DEFAULT,"set asn debug %d\n",asn_debug_switch);
	return reply;	

}


/*****************************************************
** DISCRIPTION:
**          show hnb list
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/
DBusMessage *iuh_dbus_show_hnb_list(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	dbus_error_init(&err);
	int i=0;
	unsigned int num = 0;
	Iuh_HNB **HNB;
	char *hnb_name = NULL;
	char *hnb_ip = NULL;
	unsigned int nameLen = 0;
	unsigned int ipLen = 0;
	iuh_syslog_debug_debug(IUH_DEFAULT,"###iuh_dbus_show_hnb_list###\n");
	int hnb_num = (HNB_NUM != 0) ? HNB_NUM : HNB_DEFAULT_NUM_AUTELAN;
	HNB = malloc(HNB_NUM*(sizeof(Iuh_HNB *)));
	hnb_name = (char*)malloc(HNB_NAME_LEN+1);
    hnb_ip = (char*)malloc(HNB_IP_LEN+1);
    
	while(i<hnb_num){
		if(IUH_HNB[i] != NULL){
			HNB[num] = IUH_HNB[i];
			num++;
		}
		i++;
	}
	iuh_syslog_debug_debug(IUH_DEFAULT,"num = %d\n",num);
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &num);

	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_STRING_AS_STRING
											DBUS_TYPE_STRING_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);

	for(i = 0; i < num; i++){			
		DBusMessageIter iter_struct;
			
		dbus_message_iter_open_container (&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);
		dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_UINT32,
					  &(HNB[i]->HNBID));

        memset(hnb_name, 0, HNB_NAME_LEN+1);
        nameLen = strnlen(HNB[i]->HNBNAME, HNB_NAME_LEN);
		memcpy(hnb_name, HNB[i]->HNBNAME, nameLen);
		dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_STRING,
					  &hnb_name);

        memset(hnb_ip, 0, HNB_IP_LEN+1);
        ipLen = strnlen(HNB[i]->HNBIP, HNB_IP_LEN);
		memcpy(hnb_ip, HNB[i]->HNBIP, ipLen);
		dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_STRING,
					  &hnb_ip);
					  
	    dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_UINT32,
					  &(HNB[i]->state));

		dbus_message_iter_close_container (&iter_array, &iter_struct);


	}
				
	dbus_message_iter_close_container (&iter, &iter_array);
	iuh_syslog_debug_debug(IUH_DEFAULT,"send hnb list ok\n");
	IUH_FREE_OBJECT(hnb_name);
	IUH_FREE_OBJECT(hnb_ip);
	IUH_FREE_OBJECT(HNB);
	iuh_syslog_debug_debug(IUH_DEFAULT,"free ok\n");
	
	return reply;	

}


/*****************************************************
** DISCRIPTION:
**          delete hnb by hnb id
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/
DBusMessage *iuh_dbus_delete_hnb_by_hnbid(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply; 
    DBusMessageIter  iter;
    DBusError err;
    dbus_error_init(&err);
    int ret = IUH_DBUS_SUCCESS;
    unsigned int HNBID = 0;

    iuh_syslog_debug_debug(IUH_DEFAULT,"### iuh_dbus_delete_hnb_by_hnbid ###\n");
    if (!(dbus_message_get_args ( msg, &err,
                                DBUS_TYPE_UINT32,&HNBID,
                                DBUS_TYPE_INVALID))){

        iuh_syslog_debug_debug(IUH_DEFAULT,"Unable to get input args\n");
                
        if (dbus_error_is_set(&err)) {
            iuh_syslog_debug_debug(IUH_DEFAULT,"%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }
    iuh_syslog_debug_debug(IUH_DEFAULT,"HNBID = %d\n",HNBID);
    
    if(HNBID > HNB_DEFAULT_NUM_AUTELAN)
        ret = HNB_ID_LARGE_THAN_MAX;

    ret = IUH_CMD_DELETE_HNB(HNBID);

    iuh_syslog_debug_debug(IUH_DEFAULT,"ret = %d\n",ret);
    
    reply = dbus_message_new_method_return(msg);
    
    dbus_message_iter_init_append (reply, &iter);
    
    dbus_message_iter_append_basic (&iter,
                                         DBUS_TYPE_UINT32,
                                         &ret);
    iuh_syslog_debug_debug(IUH_DEFAULT,"send ok\n");  

    return reply;	
}


/*****************************************************
** DISCRIPTION:
**          show hnb information by hnbid
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/
DBusMessage *iuh_dbus_show_hnb_info_by_hnbid(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply; 
    DBusMessageIter  iter;
	DBusMessageIter  iter_array;
    DBusError err;
    dbus_error_init(&err);
    int ret = IUH_DBUS_SUCCESS;
    unsigned int HNBID = 0;
    int macro_flag = 0;
	int gographical_flag = 0;
	char *hnbname = NULL;
	char *ifname = NULL;
	char *hnbip = NULL;
	char *hnbIdentity = NULL;
	size_t ifLen = 0;
	size_t identityLen = 0;
	size_t nameLen = 0;
	size_t ipLen = 0;
    iuh_syslog_debug_debug(IUH_DEFAULT,"### iuh_dbus_show_hnb_info_by_hnbid ###\n");
    if (!(dbus_message_get_args ( msg, &err,
                                DBUS_TYPE_UINT32,&HNBID,
                                DBUS_TYPE_INVALID))){

        iuh_syslog_err("Unable to get input args\n");
                
        if (dbus_error_is_set(&err)) {
            iuh_syslog_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }
    iuh_syslog_debug_debug(IUH_DEFAULT,"HNBID = %d\n",HNBID);
    
    if(HNBID >= HNB_DEFAULT_NUM_AUTELAN)
		ret = HNB_ID_LARGE_THAN_MAX;
	else if(IUH_HNB[HNBID] == NULL)
		ret = HNB_ID_INVALID;

	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret);
										 
	if(ret == 0){
	
	    nameLen = strnlen(IUH_HNB[HNBID]->HNBNAME, HNB_NAME_LEN);
		hnbname = malloc(nameLen+1);
		memset(hnbname, 0, nameLen+1);
		memcpy(hnbname, IUH_HNB[HNBID]->HNBNAME, nameLen);
	    dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_STRING,
										 &hnbname);
										 
		ipLen = strnlen(IUH_HNB[HNBID]->HNBIP, HNB_IP_LEN);
		hnbip = malloc(ipLen+1);
		memset(hnbip, 0, ipLen+1);
		memcpy(hnbip, IUH_HNB[HNBID]->HNBIP, ipLen);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_STRING,
										 &hnbip);
										 
		ifLen = strnlen((char*)IUH_HNB[HNBID]->BindingIFName,IF_NAME_LEN);
		ifname = malloc(ifLen+1);
		memset(ifname,0,ifLen+1);
		memcpy(ifname, IUH_HNB[HNBID]->BindingIFName, ifLen);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_STRING,
										 &ifname);
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(IUH_HNB[HNBID]->BindingSystemIndex));
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(IUH_HNB[HNBID]->HNBID));
										 
		identityLen = strnlen(IUH_HNB[HNBID]->HnbIdentity, HNB_IDENTITY_LEN);
		hnbIdentity = malloc(identityLen+1);
		memset(hnbIdentity, 0, identityLen+1);
		memcpy(hnbIdentity, IUH_HNB[HNBID]->HnbIdentity, identityLen);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_STRING,
										 &hnbIdentity);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(IUH_HNB[HNBID]->accessmode));
		//iuh_syslog_debug_debug(IUH_DEFAULT,"cellId = %d\n",IUH_HNB[HNBID]->cellId);
		dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB[HNBID]->cellId[0]));
		dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB[HNBID]->cellId[1]));
		dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB[HNBID]->cellId[2]));
		dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB[HNBID]->cellId[3]));
		
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,
										 &(IUH_HNB[HNBID]->current_UE_number));
		
		dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB[HNBID]->lac[0]));
		dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB[HNBID]->lac[1]));
		 
		dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB[HNBID]->rac[0]));
		
		dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB[HNBID]->sac[0]));
		dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB[HNBID]->sac[1]));
		
		dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB[HNBID]->plmnId[0]));
		dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB[HNBID]->plmnId[1]));
		dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB[HNBID]->plmnId[2]));
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT16,
										 &(IUH_HNB[HNBID]->rncId));
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(IUH_HNB[HNBID]->socket));
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(IUH_HNB[HNBID]->state));
										 
		if(IUH_HNB[HNBID]->HnbLocationInfo.macroCoverageInfo != NULL){
		    macro_flag = 1;
		}
		if(IUH_HNB[HNBID]->HnbLocationInfo.gographicalLocation != NULL){
		    gographical_flag = 1;
		}
        dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(macro_flag));
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(gographical_flag));
										 
		if(macro_flag){
		    dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(IUH_HNB[HNBID]->HnbLocationInfo.macroCoverageInfo->present));
			switch(IUH_HNB[HNBID]->HnbLocationInfo.macroCoverageInfo->present){
                case uTRANCellID: {
                    struct  utranCellID my_ucid = IUH_HNB[HNBID]->HnbLocationInfo.macroCoverageInfo->choice.uTRANCellID;
                    dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(my_ucid.lAC[0]));
                    dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(my_ucid.lAC[1]));
        		    dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(my_ucid.rAC[0]));
        			dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(my_ucid.pLMNidentity[0]));
        			dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(my_ucid.pLMNidentity[1]));
        			dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(my_ucid.pLMNidentity[2]));
                    dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(my_ucid.cellIdentity[0]));
                    dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(my_ucid.cellIdentity[1]));
                    dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(my_ucid.cellIdentity[2]));
                    dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(my_ucid.cellIdentity[3]));
                    break;
                }
                case gERANCellID:{
                    struct  cgi  my_cgi = IUH_HNB[HNBID]->HnbLocationInfo.macroCoverageInfo->choice.gERANCellID;
                    dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(my_cgi.pLMNidentity[0]));
        			dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(my_cgi.pLMNidentity[1]));
        			dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(my_cgi.pLMNidentity[2]));
        			dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(my_cgi.lAC[0]));
                    dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(my_cgi.lAC[1]));
                    dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(my_cgi.cI[0]));
                    dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(my_cgi.cI[1]));
                    break;
                }
                
                default:
                    break;
			}
		}
		if(gographical_flag){
		    struct  sGeographicalCoordinates my_geos = IUH_HNB[HNBID]->HnbLocationInfo.gographicalLocation->GeographicalCoordinates;
		    struct  sAltitudeAndDirection my_alt = IUH_HNB[HNBID]->HnbLocationInfo.gographicalLocation->AltitudeAndDirection;
		    dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(my_geos.LatitudeSign));
		    dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(my_geos.latitude));
			dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(my_geos.longitude));
            dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(my_alt.DirectionOfAltitude));
		    dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT16,
										 &(my_alt.altitude));
		}
		unsigned int imsi_allowed_num = 0;
		IMSIWHITELIST* head = IUH_HNB[HNBID]->imsi_white_list;
		while(head!=NULL)
		{
			imsi_allowed_num++;
			head = head->next;
		}
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(imsi_allowed_num));
		if(imsi_allowed_num)
		{
			dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);
			int i = imsi_allowed_num;
			head = IUH_HNB[HNBID]->imsi_white_list;
			while(head && imsi_allowed_num)
			{
				DBusMessageIter iter_struct;
			
				dbus_message_iter_open_container (&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);
				dbus_message_iter_append_basic (&iter_struct,
										 DBUS_TYPE_BYTE,
										 &(head->imsi[0]));
				dbus_message_iter_append_basic (&iter_struct,
										 DBUS_TYPE_BYTE,
										 &(head->imsi[1]));
				dbus_message_iter_append_basic (&iter_struct,
										 DBUS_TYPE_BYTE,
										 &(head->imsi[2]));
				dbus_message_iter_append_basic (&iter_struct,
										 DBUS_TYPE_BYTE,
										 &(head->imsi[3]));
				dbus_message_iter_append_basic (&iter_struct,
										 DBUS_TYPE_BYTE,
										 &(head->imsi[4]));
				dbus_message_iter_append_basic (&iter_struct,
										 DBUS_TYPE_BYTE,
										 &(head->imsi[5]));
				dbus_message_iter_append_basic (&iter_struct,
										 DBUS_TYPE_BYTE,
										 &(head->imsi[6]));
				dbus_message_iter_append_basic (&iter_struct,
										 DBUS_TYPE_BYTE,
										 &(head->imsi[7]));
				dbus_message_iter_close_container (&iter_array, &iter_struct);
				head = head->next;
				imsi_allowed_num--;
			}
			dbus_message_iter_close_container (&iter, &iter_array);
		}
					
	}
	IUH_FREE_OBJECT(ifname);
	IUH_FREE_OBJECT(hnbip);
	IUH_FREE_OBJECT(hnbIdentity);
	IUH_FREE_OBJECT(hnbname);

    return reply;   

}



/*****************************************************
** DISCRIPTION:
**          show ue list
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/
DBusMessage *iuh_dbus_show_ue_list(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	dbus_error_init(&err);
	int i=0;
	unsigned int num = 0;
	Iuh_HNB_UE **UE;
	iuh_syslog_debug_debug(IUH_DEFAULT,"###iuh_dbus_show_ue_list###\n");
	
	UE = malloc(UE_NUM*(sizeof(Iuh_HNB_UE *)));
	while(i<UE_NUM){
		if(IUH_HNB_UE[i] != NULL){
			UE[num] = IUH_HNB_UE[i];
			num++;
		}
		i++;
	}
	iuh_syslog_debug_debug(IUH_DEFAULT,"num = %d\n",num);
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &num);

	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);

	for(i = 0; i < num; i++){			
		DBusMessageIter iter_struct;
			
		dbus_message_iter_open_container (&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);
		dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_UINT32,
					  &(UE[i]->UEID));
			
		dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_UINT32,
					  &(UE[i]->HNBID));

		dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_UINT32,
					  &(UE[i]->state));
					  
	    dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_UINT32,
					  &(UE[i]->registrationCause));

		dbus_message_iter_close_container (&iter_array, &iter_struct);


	}
				
	dbus_message_iter_close_container (&iter, &iter_array);
	iuh_syslog_debug_debug(IUH_DEFAULT,"send ue list ok\n");
				
	IUH_FREE_OBJECT(UE);
	iuh_syslog_debug_debug(IUH_DEFAULT,"free ok\n");
	
	return reply;	

}


/*****************************************************
** DISCRIPTION:
**          delete ue by ueid
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/
DBusMessage *iuh_dbus_delete_ue_by_ueid(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply; 
    DBusMessageIter  iter;
    DBusError err;
    dbus_error_init(&err);
    int ret = IUH_DBUS_SUCCESS;
    unsigned int UEID = 0;

    iuh_syslog_debug_debug(IUH_DEFAULT,"### iuh_dbus_delete_ue_by_ueid ###\n");
    if (!(dbus_message_get_args ( msg, &err,
                                DBUS_TYPE_UINT32,&UEID,
                                DBUS_TYPE_INVALID))){

        iuh_syslog_debug_debug(IUH_DEFAULT,"Unable to get input args\n");
                
        if (dbus_error_is_set(&err)) {
            iuh_syslog_debug_debug(IUH_DEFAULT,"%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }
    iuh_syslog_debug_debug(IUH_DEFAULT,"UEID = %d\n",UEID);
    
    if(UEID > UE_NUM)
        ret = UE_ID_LARGE_THAN_MAX;

    ret = IUH_CMD_DELETE_UE(UEID);

    iuh_syslog_debug_debug(IUH_DEFAULT,"ret = %d\n",ret);
    
    reply = dbus_message_new_method_return(msg);
        
    dbus_message_iter_init_append (reply, &iter);
    
    dbus_message_iter_append_basic (&iter,
                                         DBUS_TYPE_UINT32,
                                         &ret);
    iuh_syslog_debug_debug(IUH_DEFAULT,"send ok\n");  

    return reply;	
}


/*****************************************************
** DISCRIPTION:
**          show ue information by ue id
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/
DBusMessage *iuh_dbus_show_ue_info_by_ueid(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply; 
    DBusMessageIter  iter;
    DBusError err;
    dbus_error_init(&err);
    int ret = IUH_DBUS_SUCCESS;
    unsigned int UEID = 0;
//    Iu_RAB *ue_rab = NULL;
    iuh_syslog_debug_debug(IUH_DEFAULT,"### iuh_dbus_show_ue_info_by_ueid ###\n");
    if (!(dbus_message_get_args ( msg, &err,
                                DBUS_TYPE_UINT32,&UEID,
                                DBUS_TYPE_INVALID))){

        iuh_syslog_debug_debug(IUH_DEFAULT,"Unable to get input args\n");
                
        if (dbus_error_is_set(&err)) {
            iuh_syslog_debug_debug(IUH_DEFAULT,"%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }
    iuh_syslog_debug_debug(IUH_DEFAULT,"UEID = %d\n",UEID);
    
    if(UEID >= UE_NUM)
		ret = UE_ID_LARGE_THAN_MAX;
	else if(IUH_HNB_UE[UEID] == NULL)
		ret = UE_ID_INVALID;

	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret);
	//iuh_syslog_debug_debug(IUH_DEFAULT,"ret = %d\n",ret);
	
	if(ret == IUH_DBUS_SUCCESS){
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(IUH_HNB_UE[UEID]->UEID));
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(IUH_HNB_UE[UEID]->HNBID));
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(IUH_HNB_UE[UEID]->registrationCause));
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(IUH_HNB_UE[UEID]->context_id));
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(IUH_HNB_UE[UEID]->UE_Identity.present));
	    switch(IUH_HNB_UE[UEID]->UE_Identity.present){
	        //iuh_syslog_debug_debug(IUH_DEFAULT,"present = %d\n",IUH_HNB_UE[UEID]->UE_Identity.present);
            case UE_Identity_iMSI: {
    	        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.imsi[0]));
    	        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.imsi[1]));
    	        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.imsi[2]));
    	        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.imsi[3]));
    	        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.imsi[4]));
    	        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.imsi[5]));
    	        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.imsi[6]));
    	        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.imsi[7]));
    	        break;
    	    }
    	    case UE_Identity_tMSILAI: {
    	        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.tmsilai.tmsi[0]));
    	        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.tmsilai.tmsi[1]));
    	        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.tmsilai.tmsi[2]));
    	        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.tmsilai.tmsi[3]));
    	        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.tmsilai.tmsi[4]));
    	        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.tmsilai.tmsi[5]));
    	        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.tmsilai.tmsi[6]));
    	        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.tmsilai.tmsi[7]));
    	        
		        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.tmsilai.lai.plmnid[0]));
		        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.tmsilai.lai.plmnid[1]));
		        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.tmsilai.lai.plmnid[2]));
		        
		        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.tmsilai.lai.lac[0]));
		        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.tmsilai.lai.lac[1]));
    	        break;
    	    }
    	    case UE_Identity_pTMSIRAI: {
 	            dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.ptmsirai.ptmsi[0]));
 	            dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.ptmsirai.ptmsi[1]));
 	            dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.ptmsirai.ptmsi[2]));
 	            dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.ptmsirai.ptmsi[3]));
 	            dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.ptmsirai.ptmsi[4]));
 	            dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.ptmsirai.ptmsi[5]));
 	            dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.ptmsirai.ptmsi[6]));
 	            dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.ptmsirai.ptmsi[7]));
 	            
		        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.ptmsirai.rai.rac[0]));
		        
		        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.ptmsirai.rai.lai.plmnid[0]));
		        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.ptmsirai.rai.lai.plmnid[1]));
		        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.ptmsirai.rai.lai.plmnid[2]));
		        
		        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.ptmsirai.rai.lai.lac[0]));
		        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.ptmsirai.rai.lai.lac[1]));
    	        break;
    	    }
    	    case UE_Identity_iMEI: {
    	        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.imei[0]));
    	        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.imei[1]));
    	        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.imei[2]));
    	        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.imei[3]));
    	        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.imei[4]));
    	        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.imei[5]));
    	        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.imei[6]));
    	        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.imei[7]));
    	        break;
    	    }
    	    case UE_Identity_eSN: {
    	        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.esn[0]));
    	        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.esn[1]));
    	        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.esn[2]));
    	        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.esn[3]));
    	        break;
    	    }
    	    case UE_Identity_iMSIDS41: {
    	        dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_STRING,
										 &(IUH_HNB_UE[UEID]->UE_Identity.choice.imsids41));
    	        break;
    	    }
    	    case UE_Identity_iMSIESN: {
    	        dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_STRING,
										 &(IUH_HNB_UE[UEID]->UE_Identity.choice.imsiesn.imsids41));
										 
		        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.imsiesn.esn[0]));
    	        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.imsiesn.esn[1]));
    	        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.imsiesn.esn[2]));
    	        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &(IUH_HNB_UE[UEID]->UE_Identity.choice.imsiesn.esn[3]));
    	        break;
    	    }
    	    case UE_Identity_tMSIDS41: {
    	        dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_STRING,
										 &(IUH_HNB_UE[UEID]->UE_Identity.choice.tmsids41));
    	        break;
    	    }
    	    default:
    	        break;
        }
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(IUH_HNB_UE[UEID]->csgMemStatus));
		//iuh_syslog_debug_debug(IUH_DEFAULT,"777777777777777\n");
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(IUH_HNB_UE[UEID]->state));
		//iuh_syslog_debug_debug(IUH_DEFAULT,"888888888\n");
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(IUH_HNB_UE[UEID]->Capabilities.accStrRelIndicator));
		//iuh_syslog_debug_debug(IUH_DEFAULT,"99999999999\n"); 
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(IUH_HNB_UE[UEID]->Capabilities.csgIndicator));

	#ifdef RAB_INFO
        
        /* rab/gtp-u information of current ue */
        rab_count = IUH_HNB_UE[UEID]->rab_count;
        dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &rab_count);
		iuh_syslog_debug_debug(IUH_DEFAULT,"rab_count = %d\n",rab_count);
		dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING //rab id
											DBUS_TYPE_BYTE_AS_STRING //nas src indicator
											DBUS_TYPE_UINT32_AS_STRING//delivery order
											DBUS_TYPE_UINT32_AS_STRING//max sdu size
											DBUS_TYPE_UINT32_AS_STRING//rab ass indicator
											DBUS_TYPE_UINT32_AS_STRING//traffic class
											DBUS_TYPE_UINT32_AS_STRING//user plane mode
											DBUS_TYPE_BYTE_AS_STRING //mode versions
											DBUS_TYPE_BYTE_AS_STRING //mode versions
											DBUS_TYPE_STRING_AS_STRING//transport addr
											DBUS_TYPE_UINT32_AS_STRING //trans assoc present
											DBUS_TYPE_BYTE_AS_STRING //gtp_tei | binding_id
											DBUS_TYPE_BYTE_AS_STRING //gtp_tei | binding_id
											DBUS_TYPE_BYTE_AS_STRING //gtp_tei | binding_id
											DBUS_TYPE_BYTE_AS_STRING //gtp_tei | binding_id
											DBUS_TYPE_UINT32_AS_STRING //service handover
                                            DBUS_TYPE_UINT32_AS_STRING //is ps domain
                                            DBUS_TYPE_UINT32_AS_STRING //data vol rpt
                                            DBUS_TYPE_UINT32_AS_STRING //dl gtp-pdu
                                            DBUS_TYPE_UINT32_AS_STRING //ul gtp-pdu
                                            DBUS_TYPE_UINT32_AS_STRING //dl n-pdu
                                            DBUS_TYPE_UINT32_AS_STRING //ul n-pdu
									DBUS_STRUCT_END_CHAR_AS_STRING,
	                                &iter_array);

	    for(i = 0; i < rab_count; i++){				  
	        ue_rab = IUH_HNB_UE[UEID]->UE_RAB;
	        if(ue_rab != NULL){
	            DBusMessageIter iter_struct;
    			char *transIP = NULL;
    			dbus_message_iter_open_container (&iter_array,
    											DBUS_TYPE_STRUCT,
    											NULL,
    											&iter_struct);
				iuh_syslog_debug_debug(IUH_DEFAULT,"aaaaa\n"); 

                dbus_message_iter_append_basic(&iter_struct,
    						  DBUS_TYPE_BYTE, &(ue_rab->RABID[0]));
                dbus_message_iter_append_basic(&iter_struct,
    						  DBUS_TYPE_BYTE, &(ue_rab->nAS_ScrIndicator[0]));
    			iuh_syslog_debug_debug(IUH_DEFAULT,"bbbbbb\n"); 			  
    			dbus_message_iter_append_basic(&iter_struct,
    						  DBUS_TYPE_UINT32, &(ue_rab->rab_para_list.deliveryOrder));
    			dbus_message_iter_append_basic(&iter_struct,
    						  DBUS_TYPE_UINT32, &(ue_rab->rab_para_list.maxSDUSize));
    			dbus_message_iter_append_basic(&iter_struct,
    						  DBUS_TYPE_UINT32, &(ue_rab->rab_para_list.rab_ass_indicator));
    			dbus_message_iter_append_basic(&iter_struct,
    						  DBUS_TYPE_UINT32, &(ue_rab->rab_para_list.traffic_class));
    			iuh_syslog_debug_debug(IUH_DEFAULT,"cccccc\n"); 			  
    		    dbus_message_iter_append_basic(&iter_struct,
    						  DBUS_TYPE_UINT32, &(ue_rab->user_plane_info.user_plane_mode));
    		    dbus_message_iter_append_basic(&iter_struct,
    						  DBUS_TYPE_BYTE, &(ue_rab->user_plane_info.up_modeVersions[0]));
    		    dbus_message_iter_append_basic(&iter_struct,
    						  DBUS_TYPE_BYTE, &(ue_rab->user_plane_info.up_modeVersions[1]));
    						  
    			transIP = (char*)malloc(TRANSPORT_LAYER_ADDR_LEN+1);
    			memset(transIP, 0, TRANSPORT_LAYER_ADDR_LEN+1);
    			memcpy(transIP, ue_rab->trans_layer_info.transport_layer_addr, TRANSPORT_LAYER_ADDR_LEN);

    			iuh_syslog_debug_debug(IUH_DEFAULT,"dddddd\n"); 
                dbus_message_iter_append_basic(&iter_struct,
                                              DBUS_TYPE_STRING, &(transIP));
                                              
    			dbus_message_iter_append_basic(&iter_struct,
    						  DBUS_TYPE_UINT32, &(ue_rab->trans_layer_info.iu_trans_assoc.present));
    			if(ue_rab->trans_layer_info.iu_trans_assoc.present == IuTransportAssociation_gTP_TEI){
    			    dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_BYTE, 
    			        &(ue_rab->trans_layer_info.iu_trans_assoc.choice.gtp_tei[0]));
    			    dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_BYTE, 
    			        &(ue_rab->trans_layer_info.iu_trans_assoc.choice.gtp_tei[1]));
    			    dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_BYTE, 
    			        &(ue_rab->trans_layer_info.iu_trans_assoc.choice.gtp_tei[2]));
    			    dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_BYTE, 
    			        &(ue_rab->trans_layer_info.iu_trans_assoc.choice.gtp_tei[3]));
    			}
    			else if(ue_rab->trans_layer_info.iu_trans_assoc.present == IuTransportAssociation_bindingID){
    			    dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_BYTE, 
    			        &(ue_rab->trans_layer_info.iu_trans_assoc.choice.binding_id[0]));
    			    dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_BYTE, 
    			        &(ue_rab->trans_layer_info.iu_trans_assoc.choice.binding_id[1]));
    			    dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_BYTE, 
    			        &(ue_rab->trans_layer_info.iu_trans_assoc.choice.binding_id[2]));
    			    dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_BYTE, 
    			        &(ue_rab->trans_layer_info.iu_trans_assoc.choice.binding_id[3]));
    			}

    			dbus_message_iter_append_basic(&iter_struct,
    						  DBUS_TYPE_UINT32, &(ue_rab->service_handover));
    			dbus_message_iter_append_basic(&iter_struct,
    						  DBUS_TYPE_UINT32, &(ue_rab->isPsDomain));
				iuh_syslog_debug_debug(IUH_DEFAULT,"eeeeee\n"); 
    			/* gtp-u  information */
    			dbus_message_iter_append_basic(&iter_struct,
    						  DBUS_TYPE_UINT32, &(ue_rab->data_vol_rpt));
    			dbus_message_iter_append_basic(&iter_struct,
    						  DBUS_TYPE_UINT32, &(ue_rab->dl_GTP_PDU_SeqNum));
    		    dbus_message_iter_append_basic(&iter_struct,
    						  DBUS_TYPE_UINT32, &(ue_rab->ul_GTP_PDU_seqNum));
    		    dbus_message_iter_append_basic(&iter_struct,
    						  DBUS_TYPE_UINT32, &(ue_rab->dl_N_PDU_SeqNum));
    			dbus_message_iter_append_basic(&iter_struct,
    						  DBUS_TYPE_UINT32, &(ue_rab->ul_N_PDU_SeqNum));

    			dbus_message_iter_close_container (&iter_array, &iter_struct);
				IUH_FREE_OBJECT(transIP);
				iuh_syslog_debug_debug(IUH_DEFAULT,"ffffff\n"); 
	        }
	        ue_rab = ue_rab->rab_next;
	    }
	    dbus_message_iter_close_container (&iter, &iter_array);
	#endif
	}

    //iuh_syslog_debug_debug(IUH_DEFAULT,"000000000000\n");
    
    return reply;   

}


/*****************************************************
 * iuh_dbus_profile_config_save
 *
 * DESCRIPTION:
 *          This function will set all config we want to save into the string
 *          we input.
 *
 * INPUT:
 *          show string
 *						
 * OUTPUT:
 *		void
 *				
 * RETURN:		
 *		void
 *
 *****************************************************/

void iuh_dbus_profile_config_save(char* showStr)
{	
	char *cursor = NULL;	
	int totalLen = 0;

	if(NULL == showStr){
		return ;
	}
	
	cursor = showStr;
	/*xiaodawei add, 20111210*/
	if((g_auto_hnb_login.ifnum != 0)||(g_auto_hnb_login.auto_hnb_if != NULL))
	{
		iuh_auto_hnb_if *iflist;
		iflist = g_auto_hnb_login.auto_hnb_if;
		while(iflist != NULL)
		{
			cursor = showStr + totalLen;
			totalLen += sprintf(cursor,"set auto_hnb_login interface add %s\n",iflist->ifname);
			cursor = showStr + totalLen;
			
			iflist = iflist->ifnext;
		}
	}
	totalLen += sprintf(cursor, "config iuh\n");
	cursor = showStr + totalLen;
	if(gIuhLOGLEVEL != IUH_SYSLOG_DEFAULT){
		if(gIuhLOGLEVEL == IUH_ALL){
			totalLen += sprintf(cursor, "set iuh daemonlog all debug open\n");
			cursor = showStr + totalLen;
		}
		else{
			if(gIuhLOGLEVEL == IUH_DEFAULT){
				totalLen += sprintf(cursor, "set iuh daemonlog default debug open\n");
				cursor = showStr + totalLen;
			}
			if(gIuhLOGLEVEL == IUH_DBUS){
				totalLen += sprintf(cursor, "set iuh daemonlog dbus debug open\n");
				cursor = showStr + totalLen;
			}
			if(gIuhLOGLEVEL == IUH_HNBINFO){
				totalLen += sprintf(cursor, "set iuh daemonlog hnbinfo debug open\n");
				cursor = showStr + totalLen;
			}
			if(gIuhLOGLEVEL == IUH_MB){
				totalLen += sprintf(cursor, "set iuh daemonlog mb debug open\n");
				cursor = showStr + totalLen;
			}
		}
	}
	if(asn_debug_switch == 1){
		totalLen += sprintf(cursor, "set asn1 debug switch open\n");
		cursor = showStr + totalLen;
	}
	if(gRncId){
	    totalLen += sprintf(cursor, "set rncid %d\n", gRncId);
		cursor = showStr + totalLen;
	}
	if(gSwitch.paging_imsi == 1){
		totalLen += sprintf(cursor, "set paging optimize by imsi open\n");
		cursor = showStr + totalLen;
	}
	if(gSwitch.paging_lai == 1){
		totalLen += sprintf(cursor, "set paging optimize by lai open\n");
		cursor = showStr + totalLen;
	}
	totalLen += sprintf(cursor, "exit\n");
	cursor = showStr + totalLen;

	return ;
}


/*****************************************************
** DISCRIPTION:
**          show running config of iuh
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

DBusMessage* 
iuh_dbus_show_running_cfg
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply; 
    DBusMessageIter  iter;
    DBusError err;   		
    dbus_error_init(&err);

    //printf("we are in function iuh_dbus_show_running_cfg\n");
    
	char *strShow = NULL;
	strShow = (char*)malloc(IUH_SAVE_CFG_MEM);	
	if(!strShow) {
		iuh_syslog_err("alloc memory fail when mirror show running-config\n");
		return NULL;
	}
	memset(strShow, 0, IUH_SAVE_CFG_MEM);

	iuh_dbus_profile_config_save(strShow);

    reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &strShow);
	
	IUH_FREE_OBJECT(strShow);
	
	return reply;
}



/*****************************************************
** DISCRIPTION:
**          set paging optimize switch
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/
DBusMessage *iuh_dbus_set_paging_optimize_switch(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusError err;
	dbus_error_init(&err);
	int ret = IUH_DBUS_SUCCESS;
	unsigned int optimize_type = 0;
	unsigned int optimize_switch = 0;

	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&optimize_type,
								DBUS_TYPE_UINT32,&optimize_switch,
								DBUS_TYPE_INVALID))){

		iuh_syslog_debug_debug(IUH_DEFAULT,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			iuh_syslog_debug_debug(IUH_DEFAULT,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if(optimize_type == 0){
	    gSwitch.paging_imsi = optimize_switch;
	    if(optimize_switch == 1)
	        gSwitch.paging_lai = 0;
	}
	else if(optimize_type == 1){
	    gSwitch.paging_lai = optimize_switch;
	    if(optimize_switch == 1)
	        gSwitch.paging_imsi = 0;
	}
	    
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);

	
	iuh_syslog_debug_debug(IUH_DEFAULT,"set paging optimize %s %s\n", (optimize_type==0)?"imsi":"lai", (optimize_switch==0)?"close":"open");
	
	return reply;	

}
DBusMessage *iuh_dbus_femto_acl_white_list(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 
	DBusMessageIter  iter;
	DBusError err;
	dbus_error_init(&err);
	int ret = IUH_DBUS_SUCCESS;
	unsigned int op_type = 0;
	unsigned int hnb_id = 0;
	unsigned char *imsi = NULL;

	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&op_type,
								DBUS_TYPE_UINT32,&hnb_id,
								DBUS_TYPE_STRING,&imsi,
								DBUS_TYPE_INVALID))){

		iuh_syslog_debug_debug(IUH_DEFAULT,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			iuh_syslog_debug_debug(IUH_DEFAULT,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(hnb_id >= HNB_DEFAULT_NUM_AUTELAN)
		ret = HNB_ID_LARGE_THAN_MAX;
	else if(IUH_HNB[hnb_id] == NULL)
		ret = HNB_ID_INVALID;
	if(ret == IUH_DBUS_SUCCESS){
		IMSIWHITELIST** head = &(IUH_HNB[hnb_id]->imsi_white_list);
		unsigned char* imsi_str = NULL;
		imsi_str = (unsigned char*)malloc(IMSI_LEN);
		IUH_IMSI_DIGIT_CONVERT(imsi, imsi_str);
		if(!op_type)
			ret = IUH_ADD_FEMTO_ACL_WHITE_LIST(head, imsi_str);
		else
			ret = IUH_DEL_FEMTO_ACL_WHITE_LIST(head, imsi_str);
		
		IUH_FREE_OBJECT(imsi_str);
	}
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);

	if(ret == IUH_DBUS_SUCCESS)
		iuh_syslog_debug_debug(IUH_DEFAULT,"%s femto acl white_list hnb %d %s\n", (op_type==0)?"add":"del", hnb_id, imsi);
	
	return reply;	

}
DBusMessage *iuh_dbus_iuh_tipc_init(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 
	DBusMessageIter  iter;
	DBusError err;
	dbus_error_init(&err);
	int ret = IUH_DBUS_SUCCESS;
	unsigned int islocal = 0;
	unsigned int iu_slotid = 0;
	unsigned int iu_insid = 0;

	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&islocal,
								DBUS_TYPE_UINT32,&iu_slotid,
								DBUS_TYPE_UINT32,&iu_insid,
								DBUS_TYPE_INVALID))){

		iuh_syslog_debug_debug(IUH_DEFAULT,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			iuh_syslog_debug_debug(IUH_DEFAULT,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(IuhServerTipcInit(iu_slotid, iu_insid, islocal) || IuhClientTipcInit(iu_slotid, iu_insid, islocal))
		ret = IUH_TIPC_SOCK_INIT_ERROR;
	if(ret == IUH_DBUS_SUCCESS)
		gIuhTipcSockFlag = 1;
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
	
	return reply;	

}


static DBusHandlerResult iuh_dbus_message_handler (DBusConnection *connection, DBusMessage *message, void *user_data){
	DBusMessage		*reply = NULL;

	if(message == NULL);

	if	(strcmp(dbus_message_get_path(message),IUH_DBUS_OBJPATH) == 0)	{
        if(dbus_message_is_method_call(message,IUH_DBUS_INTERFACE,IUH_DBUS_CONF_METHOD_SET_IUH_DYNAMIC_HNB_LOGIN_INTERFACE)) {
    		reply = iuh_dbus_dynamic_hnb_login_ifname(connection,message,user_data);
    	}  
    	else if(dbus_message_is_method_call(message,IUH_DBUS_INTERFACE,IUH_DBUS_SECURITY_METHOD_SET_IUH_DAEMONLOG_DEBUG)) {
    		reply = iuh_dbus_set_iuh_daemonlog_debug(connection,message,user_data);
    	} 
    	else if(dbus_message_is_method_call(message,IUH_DBUS_INTERFACE,IUH_DBUS_HNB_METHOD_SHOW_HNBINFO_BY_HNBID)) {
    		reply = iuh_dbus_show_hnb_info_by_hnbid(connection,message,user_data);
    	}
    	else if(dbus_message_is_method_call(message,IUH_DBUS_INTERFACE,IUH_DBUS_HNB_METHOD_SHOW_HNB_LIST)) {
    		reply = iuh_dbus_show_hnb_list(connection,message,user_data);
    	}
        else if(dbus_message_is_method_call(message,IUH_DBUS_INTERFACE,IUH_DBUS_SECURITY_METHOD_DELETE_HNB_BY_HNBID)) {
    		reply = iuh_dbus_delete_hnb_by_hnbid(connection,message,user_data);
    	}
    	else if(dbus_message_is_method_call(message,IUH_DBUS_INTERFACE,IUH_DBUS_HNB_METHOD_SHOW_UEINFO_BY_UEID)) {
    		reply = iuh_dbus_show_ue_info_by_ueid(connection,message,user_data);
    	}
    	else if(dbus_message_is_method_call(message,IUH_DBUS_INTERFACE,IUH_DBUS_HNB_METHOD_SHOW_UE_LIST)) {
    		reply = iuh_dbus_show_ue_list(connection,message,user_data);
    	}
        else if(dbus_message_is_method_call(message,IUH_DBUS_INTERFACE,IUH_DBUS_SECURITY_METHOD_DELETE_UE_BY_UEID)) {
    		reply = iuh_dbus_delete_ue_by_ueid(connection,message,user_data);
    	}
    	else if(dbus_message_is_method_call(message,IUH_DBUS_INTERFACE,IUH_DBUS_IUH_SET_ASN_DEBUG_SWITCH)){
    	    reply = iuh_dbus_set_iuh_asn_debug(connection,message,user_data);
    	}
    	else if(dbus_message_is_method_call(message,IUH_DBUS_INTERFACE,IUH_DBUS_IUH_SET_RNCID)){
    	    reply = iuh_dbus_set_rncid(connection,message,user_data);
    	}
    	else if(dbus_message_is_method_call(message,IUH_DBUS_INTERFACE,IUH_DBUS_METHOD_SHOW_RUNNING_CFG)) {
		    reply = iuh_dbus_show_running_cfg(connection,message,user_data);
	    }
	    else if(dbus_message_is_method_call(message,IUH_DBUS_INTERFACE,IUH_SET_PAGING_OPTIMIZE_SWITCH)) {
		    reply = iuh_dbus_set_paging_optimize_switch(connection,message,user_data);
	    }
		else if(dbus_message_is_method_call(message,IUH_DBUS_INTERFACE,IUH_FEMTO_ACL_WHITE_LIST)) {
		    reply = iuh_dbus_femto_acl_white_list(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,IUH_DBUS_INTERFACE,IUH_DBUS_IUH_TIPC_INIT)) {
		    reply = iuh_dbus_iuh_tipc_init(connection,message,user_data);
		}
	}
	if (reply) {
		dbus_connection_send (connection, reply, NULL);
		dbus_connection_flush(connection); 
		dbus_message_unref (reply);
	}

	return DBUS_HANDLER_RESULT_HANDLED ;
}



DBusHandlerResult
iuh_dbus_filter_function (DBusConnection * connection,
					   DBusMessage * message, void *user_data)
{
	if (dbus_message_is_signal (message, DBUS_INTERFACE_LOCAL, "Disconnected") &&
		   strcmp (dbus_message_get_path (message), DBUS_PATH_LOCAL) == 0) {

		iuh_syslog_err("Got disconnected from the system message bus; "
				"retrying to reconnect every 3000 ms\n");
		dbus_connection_unref (iuh_dbus_connection);
		iuh_dbus_connection = NULL;
		IuhThread thread_dbus; 
		if(!(IuhCreateThread(&thread_dbus, iuh_dbus_thread_restart, NULL,0))) {
			iuh_syslog_crit("Error starting Dbus Thread");
			exit(1);
		}

	} else if (dbus_message_is_signal (message,
			      DBUS_INTERFACE_DBUS,
			      "NameOwnerChanged")) {

	} else
		return TRUE;

	return DBUS_HANDLER_RESULT_HANDLED;
}

int iuh_dbus_reinit(void)
{	
	int i = 0;
	DBusError dbus_error;
	dbus_threads_init_default();
	
	DBusObjectPathVTable	iuh_vtable = {NULL, &iuh_dbus_message_handler, NULL, NULL, NULL, NULL};	

	iuh_syslog_debug_debug(IUH_DEFAULT,"iuh dbus init\n");

	dbus_connection_set_change_sigpipe (TRUE);

	dbus_error_init (&dbus_error);
	iuh_dbus_connection = dbus_bus_get_private (DBUS_BUS_SYSTEM, &dbus_error);
	if (iuh_dbus_connection == NULL) {
		iuh_syslog_err("dbus_bus_get(): %s\n", dbus_error.message);
		return FALSE;
	}

	// Use npd to handle subsection of NPD_DBUS_OBJPATH including slots
	if (!dbus_connection_register_fallback (iuh_dbus_connection, IUH_DBUS_OBJPATH, &iuh_vtable, NULL)) {
		iuh_syslog_err("can't register D-BUS handlers (fallback NPD). cannot continue.\n");
		return FALSE;
		
	}
	
	
	i = dbus_bus_request_name (iuh_dbus_connection, IUH_DBUS_BUSNAME,
			       0, &dbus_error);
		
	iuh_syslog_debug_debug(IUH_DBUS,"dbus_bus_request_name:%d",i);
	
	if (dbus_error_is_set (&dbus_error)) {
		iuh_syslog_debug_debug(IUH_DBUS,"dbus_bus_request_name(): %s",
			    dbus_error.message);
		return FALSE;
	}

	dbus_connection_add_filter (iuh_dbus_connection, iuh_dbus_filter_function, NULL, NULL);

	dbus_bus_add_match (iuh_dbus_connection,
			    "type='signal'"
					    ",interface='"DBUS_INTERFACE_DBUS"'"
					    ",sender='"DBUS_SERVICE_DBUS"'"
					    ",member='NameOwnerChanged'",
			    NULL);
	
	return TRUE;
  
}

int iuh_dbus_init(void)
{	
	int i = 0;
	DBusError dbus_error;
	dbus_threads_init_default();
	
	DBusObjectPathVTable	iuh_vtable = {NULL, &iuh_dbus_message_handler, NULL, NULL, NULL, NULL};	

	dbus_connection_set_change_sigpipe (TRUE);

	dbus_error_init (&dbus_error);
	iuh_dbus_connection = dbus_bus_get_private (DBUS_BUS_SYSTEM, &dbus_error);

	if (iuh_dbus_connection == NULL) {
		iuh_syslog_err("dbus_bus_get(): %s\n", dbus_error.message);
		return FALSE;
	}

	if (!dbus_connection_register_fallback (iuh_dbus_connection, IUH_DBUS_OBJPATH, &iuh_vtable, NULL)) {
		iuh_syslog_err("can't register D-BUS handlers (fallback NPD). cannot continue.\n");
		return FALSE;
		
	}
	
	i = dbus_bus_request_name (iuh_dbus_connection, IUH_DBUS_BUSNAME,
			       0, &dbus_error);
	
	if (dbus_error_is_set (&dbus_error)) {
		iuh_syslog_debug_debug(IUH_DBUS,"dbus_bus_request_name(): %s",
			    dbus_error.message);
		return FALSE;
	}

	dbus_connection_add_filter (iuh_dbus_connection, iuh_dbus_filter_function, NULL, NULL);

	dbus_bus_add_match (iuh_dbus_connection,
			    "type='signal'"
					    ",interface='"DBUS_INTERFACE_DBUS"'"
					    ",sender='"DBUS_SERVICE_DBUS"'"
					    ",member='NameOwnerChanged'",
			    NULL);

    //iuh_syslog_debug_debug(IUH_DEFAULT, "init finished\n");
	return TRUE;
  
}

void *iuh_dbus_thread()
{
    iuh_syslog_debug_debug(IUH_DEFAULT,"iuh_dbus_thread\n");
	if(iuh_dbus_init()&&
	   IuhGetMsgQueue(&IuhDBUS_MSGQ)
	){
		while (dbus_connection_read_write_dispatch(iuh_dbus_connection,500)) {
		
		}
	}
	iuh_syslog_err("there is something wrong in dbus handler\n");	

	return 0;
}

void *iuh_dbus_thread_restart()
{
	if(iuh_dbus_reinit()&&
	   IuhGetMsgQueue(&IuhDBUS_MSGQ)
	){
		while (dbus_connection_read_write_dispatch(iuh_dbus_connection,500)) {

		}
	}
	iuh_syslog_err("there is something wrong in dbus handler\n");	
	return 0;
}



