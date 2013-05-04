
#include <string.h>
#include <dbus/dbus.h>
#include "../../accapi/dbus/iuh/IuhDBusDef.h"
#include "../../accapi/dbus/iu/IuDbusDef.h"
#include "../../accapi/dbus/iu/SigtranDbusDef.h"
#include "../../accapi/dbus/iuh/IuhDBusPath.h"
#include "../../accapi/iuh/Iuh.h"
#include "../../accapi/iuh/mapi_hnb.h"
#include "../../accapi/dbus/hmd/HmdDbusDef.h"
#include "mapi_hnb.h"

void ReInitFemtoDbusPath(int local, int index, char * path, char * newpath)
{
	int len;
//#ifndef _DISTRIBUTION_
		//sprintf(newpath,"%s%d",path,index);
//#else
		sprintf(newpath,"%s%d_%d",path,local,index);
//#endif
	if(path == IUH_DBUS_OBJPATH){
		len = strlen(newpath);
		sprintf(newpath+len, "/%s", "iuh");
	}
	else if(path == IUH_DBUS_INTERFACE){
		len = strlen(newpath);
		sprintf(newpath+len, ".%s", "iuh");
	}
	else if(path == IU_DBUS_OBJPATH){
		len = strlen(newpath);
		sprintf(newpath+len, "/%s", "iu");
	}
	else if(path == IU_DBUS_INTERFACE){
		len = strlen(newpath);
		sprintf(newpath+len, ".%s", "iu");
	}else if(path == UDP_IU_DBUS_OBJPATH){
		len = strlen(newpath);
		sprintf(newpath+len, "/%s", "sigtran");
	}
	else if(path == UDP_IU_DBUS_INTERFACE){
		len = strlen(newpath);
		sprintf(newpath+len, ".%s", "sigtran");
	}
}

void dcli_free_hnb_info(Iuh_HNB* HNBINFO)
{
	if(HNBINFO == NULL)
		return;
	IUH_FREE_OBJECT(HNBINFO->HnbLocationInfo.gographicalLocation);
    IUH_FREE_OBJECT(HNBINFO->HnbLocationInfo.macroCoverageInfo);
	
	IMSIWHITELIST* head = HNBINFO->imsi_white_list;
	IMSIWHITELIST* tmp = NULL;
	while(head != NULL)
	{
		tmp = head;
		head = head->next;
		IUH_FREE_OBJECT(tmp);
	}
	
    IUH_FREE_OBJECT(HNBINFO);
	return;
}
void dcli_free_ue_list(UELIST *uelist)
{
    if(uelist == NULL) return;
    int i = 0;
    int num = uelist->uecount;
    for(i = 0; i < num; i++){
        IUH_FREE_OBJECT(uelist->UE_LIST[i]);
    }
    IUH_FREE_OBJECT(uelist);
    return;
}

/*****************************************************
** DISCRIPTION:
**          bind interface for hnb
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

int mapi_iuh_auto_hnb_login(int index, int localid, char *name, unsigned char policy, DBusConnection *dbus_connection, char *DBUS_METHOD)
{
    DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int ret = 0;
	
    char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitFemtoDbusPath(localid,index,IUH_DBUS_BUSNAME,BUSNAME);
	ReInitFemtoDbusPath(localid,index,IUH_DBUS_OBJPATH,OBJPATH);
	ReInitFemtoDbusPath(localid,index,IUH_DBUS_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&policy,
							 DBUS_TYPE_STRING,&name,
							 DBUS_TYPE_INVALID);
    
	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_unref(reply);

	return ret;

}


/*****************************************************
** DISCRIPTION:
**          set asn1 debug information switch open/close
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

int mapi_set_asn_debug_switch(int index, int localid, unsigned int debug_switch, DBusConnection *dbus_connection, char *DBUS_METHOD)
{
    DBusMessage *query, *reply; 
    DBusMessageIter  iter;
    DBusError err;
    int ret = 0;
    
    char BUSNAME[PATH_LEN];
    char OBJPATH[PATH_LEN];
    char INTERFACE[PATH_LEN];
    ReInitFemtoDbusPath(localid,index,IUH_DBUS_BUSNAME,BUSNAME);
    ReInitFemtoDbusPath(localid,index,IUH_DBUS_OBJPATH,OBJPATH);
    ReInitFemtoDbusPath(localid,index,IUH_DBUS_INTERFACE,INTERFACE);
    
    query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
    
    dbus_error_init(&err);

    dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&debug_switch,
                             DBUS_TYPE_INVALID);
    
    reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
    
    dbus_message_unref(query);
    
    if (NULL == reply) {
        printf("<error> failed get reply.\n");
        if (dbus_error_is_set(&err)) {
            printf("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return -1;
    }
    
    dbus_message_iter_init(reply,&iter);
    dbus_message_iter_get_basic(&iter,&ret);

    dbus_message_unref(reply);

    return ret;

}



/*****************************************************
** DISCRIPTION:
**          set paging IMSI/LAI optimize switch open/close
** INPUT:
**          optimize flag
**          optimize switch
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

int mapi_set_paging_optimize_switch(int index, int localid, unsigned int optimize_type, unsigned int optimize_switch, DBusConnection *dbus_connection, char *DBUS_METHOD)
{
    DBusMessage *query, *reply; 
    DBusMessageIter  iter;
    DBusError err;
    int ret = 0;
    
    char BUSNAME[PATH_LEN];
    char OBJPATH[PATH_LEN];
    char INTERFACE[PATH_LEN];
    ReInitFemtoDbusPath(localid,index,IUH_DBUS_BUSNAME,BUSNAME);
    ReInitFemtoDbusPath(localid,index,IUH_DBUS_OBJPATH,OBJPATH);
    ReInitFemtoDbusPath(localid,index,IUH_DBUS_INTERFACE,INTERFACE);
    
    query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
    
    dbus_error_init(&err);

    dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&optimize_type,
                             DBUS_TYPE_UINT32,&optimize_switch,
                             DBUS_TYPE_INVALID);
    
    reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
    
    dbus_message_unref(query);
    
    if (NULL == reply) {
        printf("<error> failed get reply.\n");
        if (dbus_error_is_set(&err)) {
            printf("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return -1;
    }
    
    dbus_message_iter_init(reply,&iter);
    dbus_message_iter_get_basic(&iter,&ret);

    dbus_message_unref(reply);

    return ret;

}



/*****************************************************
** DISCRIPTION:
**          set RncId
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

int mapi_set_rncid(int index, int localid, unsigned short int rncid, DBusConnection *dbus_connection, char *DBUS_METHOD)
{
    DBusMessage *query, *reply; 
    DBusMessageIter  iter;
    DBusError err;
    int ret = 0;
    
    char BUSNAME[PATH_LEN];
    char OBJPATH[PATH_LEN];
    char INTERFACE[PATH_LEN];
    ReInitFemtoDbusPath(localid,index,IUH_DBUS_BUSNAME,BUSNAME);
    ReInitFemtoDbusPath(localid,index,IUH_DBUS_OBJPATH,OBJPATH);
    ReInitFemtoDbusPath(localid,index,IUH_DBUS_INTERFACE,INTERFACE);
    
    query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
    
    dbus_error_init(&err);

    dbus_message_append_args(query,
                             DBUS_TYPE_UINT16,&rncid,
                             DBUS_TYPE_INVALID);
    
    reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
    
    dbus_message_unref(query);
    
    if (NULL == reply) {
        printf("<error> failed get reply.\n");
        if (dbus_error_is_set(&err)) {
            printf("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return -1;
    }
    
    dbus_message_iter_init(reply,&iter);
    dbus_message_iter_get_basic(&iter,&ret);

    dbus_message_unref(reply);

    return ret;

}


/*****************************************************
** DISCRIPTION:
**          set iuh daemon log level
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

int mapi_set_iuh_daemonlog(int index, int localid, unsigned int daemonlogtype, unsigned int daemonloglevel, DBusConnection *dbus_connection, char *DBUS_METHOD)
{
    DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int ret = 0;
	
    char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitFemtoDbusPath(localid,index,IUH_DBUS_BUSNAME,BUSNAME);
	ReInitFemtoDbusPath(localid,index,IUH_DBUS_OBJPATH,OBJPATH);
	ReInitFemtoDbusPath(localid,index,IUH_DBUS_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
	
	dbus_error_init(&err);

    dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&daemonlogtype,
							 DBUS_TYPE_UINT32,&daemonloglevel,
							 DBUS_TYPE_INVALID);
    
	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply){
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)){
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_unref(reply);

	return ret;

}


/*****************************************************
** DISCRIPTION:
**          delete hnb information by hnb id
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

int mapi_delete_hnb_id(int index, int localid, unsigned int hnbid, DBusConnection *dbus_connection, char *DBUS_METHOD)
{
    DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int ret = 0;
	
    char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitFemtoDbusPath(localid,index,IUH_DBUS_BUSNAME,BUSNAME);
	ReInitFemtoDbusPath(localid,index,IUH_DBUS_OBJPATH,OBJPATH);
	ReInitFemtoDbusPath(localid,index,IUH_DBUS_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&hnbid,
							 DBUS_TYPE_INVALID);
    
	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply){
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)){
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_unref(reply);

	return ret;

}


/*****************************************************
** DISCRIPTION:
**          get hnb information from iuhd by hnbid 
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

Iuh_HNB * mapi_get_hnb_info_by_hnbid(int index, int localid, unsigned int hnb_id, int *ret, DBusConnection *dbus_connection, char *DBUS_METHOD)
{
    DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	iter_array;
	DBusError err;

	Iuh_HNB *hnbInfo = NULL;
	char *hnbidentity = NULL;
	char *hnbname = NULL;
	char *hnbifname = NULL;
	char *hnbip = NULL;
	int macro_flag;
	int gographical_flag;
	
    char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitFemtoDbusPath(localid,index,IUH_DBUS_BUSNAME,BUSNAME);
	ReInitFemtoDbusPath(localid,index,IUH_DBUS_OBJPATH,OBJPATH);
	ReInitFemtoDbusPath(localid,index,IUH_DBUS_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&hnb_id,
							 DBUS_TYPE_INVALID);
							 
	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply){
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)){
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
	
    dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){
	    hnbInfo = (Iuh_HNB*)malloc(sizeof(Iuh_HNB));
    	memset(hnbInfo, 0, sizeof(Iuh_HNB));
		hnbInfo->imsi_white_list = NULL;
		IMSIWHITELIST* head = hnbInfo->imsi_white_list;
    	dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(hnbname));
    	memcpy(hnbInfo->HNBNAME, hnbname, strnlen(hnbname, HNB_NAME_LEN));	

    	dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(hnbip));
    	memcpy(hnbInfo->HNBIP, hnbip, strnlen(hnbip, HNB_IP_LEN));
    	
        dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(hnbifname));
    	memcpy(hnbInfo->BindingIFName, hnbifname, strnlen(hnbifname, IF_NAME_LEN));
    	
        dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(hnbInfo->BindingSystemIndex));
        dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(hnbInfo->HNBID));
        dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(hnbidentity));
    	memcpy(hnbInfo->HnbIdentity, hnbidentity, strnlen(hnbidentity, HNB_IDENTITY_LEN));
        dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(hnbInfo->accessmode));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(hnbInfo->cellId[0]));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(hnbInfo->cellId[1]));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(hnbInfo->cellId[2]));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(hnbInfo->cellId[3]));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(hnbInfo->current_UE_number));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(hnbInfo->lac[0]));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(hnbInfo->lac[1]));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(hnbInfo->rac[0]));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(hnbInfo->sac[0]));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(hnbInfo->sac[1]));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(hnbInfo->plmnId[0]));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(hnbInfo->plmnId[1]));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(hnbInfo->plmnId[2]));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(hnbInfo->rncId));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(hnbInfo->socket));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(hnbInfo->state));
        dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(macro_flag));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(gographical_flag));
        if(macro_flag){
		    hnbInfo->HnbLocationInfo.macroCoverageInfo = (struct  macroCellID *)malloc(sizeof(struct  macroCellID));
		    dbus_message_iter_next(&iter);	
		    dbus_message_iter_get_basic(&iter,&(hnbInfo->HnbLocationInfo.macroCoverageInfo->present));
		    switch(hnbInfo->HnbLocationInfo.macroCoverageInfo->present){
		        case uTRANCellID: {
                    dbus_message_iter_next(&iter);	
            		dbus_message_iter_get_basic(&iter,&(hnbInfo->HnbLocationInfo.macroCoverageInfo->choice.uTRANCellID.lAC[0]));
            		dbus_message_iter_next(&iter);	
            		dbus_message_iter_get_basic(&iter,&(hnbInfo->HnbLocationInfo.macroCoverageInfo->choice.uTRANCellID.lAC[1]));
            		
            		dbus_message_iter_next(&iter);	
            		dbus_message_iter_get_basic(&iter,&(hnbInfo->HnbLocationInfo.macroCoverageInfo->choice.uTRANCellID.rAC[0]));
            		
            		dbus_message_iter_next(&iter);	
            		dbus_message_iter_get_basic(&iter,&(hnbInfo->HnbLocationInfo.macroCoverageInfo->choice.uTRANCellID.pLMNidentity[0]));
            		dbus_message_iter_next(&iter);	
            		dbus_message_iter_get_basic(&iter,&(hnbInfo->HnbLocationInfo.macroCoverageInfo->choice.uTRANCellID.pLMNidentity[1]));
            		dbus_message_iter_next(&iter);	
            		dbus_message_iter_get_basic(&iter,&(hnbInfo->HnbLocationInfo.macroCoverageInfo->choice.uTRANCellID.pLMNidentity[2]));
            		
            		dbus_message_iter_next(&iter);	
            		dbus_message_iter_get_basic(&iter,&(hnbInfo->HnbLocationInfo.macroCoverageInfo->choice.uTRANCellID.cellIdentity[0]));
            		dbus_message_iter_next(&iter);	
            		dbus_message_iter_get_basic(&iter,&(hnbInfo->HnbLocationInfo.macroCoverageInfo->choice.uTRANCellID.cellIdentity[1]));
            		dbus_message_iter_next(&iter);	
            		dbus_message_iter_get_basic(&iter,&(hnbInfo->HnbLocationInfo.macroCoverageInfo->choice.uTRANCellID.cellIdentity[2]));
            		dbus_message_iter_next(&iter);	
            		dbus_message_iter_get_basic(&iter,&(hnbInfo->HnbLocationInfo.macroCoverageInfo->choice.uTRANCellID.cellIdentity[3]));
                    break;
                }
                case gERANCellID: {
        			dbus_message_iter_next(&iter);	
            		dbus_message_iter_get_basic(&iter,&(hnbInfo->HnbLocationInfo.macroCoverageInfo->choice.gERANCellID.pLMNidentity[0]));
            		dbus_message_iter_next(&iter);	
            		dbus_message_iter_get_basic(&iter,&(hnbInfo->HnbLocationInfo.macroCoverageInfo->choice.gERANCellID.pLMNidentity[1]));
            		dbus_message_iter_next(&iter);	
            		dbus_message_iter_get_basic(&iter,&(hnbInfo->HnbLocationInfo.macroCoverageInfo->choice.gERANCellID.pLMNidentity[2]));
            		
            		dbus_message_iter_next(&iter);	
            		dbus_message_iter_get_basic(&iter,&(hnbInfo->HnbLocationInfo.macroCoverageInfo->choice.gERANCellID.lAC[0]));
            		dbus_message_iter_next(&iter);	
            		dbus_message_iter_get_basic(&iter,&(hnbInfo->HnbLocationInfo.macroCoverageInfo->choice.gERANCellID.lAC[1]));
            		
            		dbus_message_iter_next(&iter);	
            		dbus_message_iter_get_basic(&iter,&(hnbInfo->HnbLocationInfo.macroCoverageInfo->choice.gERANCellID.cI[0]));
            		dbus_message_iter_next(&iter);	
            		dbus_message_iter_get_basic(&iter,&(hnbInfo->HnbLocationInfo.macroCoverageInfo->choice.gERANCellID.cI[1]));
                    break;
                }
                default:
                    break;
		    }
		}
		if(gographical_flag){
		    hnbInfo->HnbLocationInfo.gographicalLocation = (struct  geographicalLocation *)malloc(sizeof(struct  geographicalLocation));
		    dbus_message_iter_next(&iter);	
    		dbus_message_iter_get_basic(&iter,&(hnbInfo->HnbLocationInfo.gographicalLocation->GeographicalCoordinates.LatitudeSign));
    		dbus_message_iter_next(&iter);	
    		dbus_message_iter_get_basic(&iter,&(hnbInfo->HnbLocationInfo.gographicalLocation->GeographicalCoordinates.latitude));
    		dbus_message_iter_next(&iter);	
    		dbus_message_iter_get_basic(&iter,&(hnbInfo->HnbLocationInfo.gographicalLocation->GeographicalCoordinates.longitude));
    		dbus_message_iter_next(&iter);	
    		dbus_message_iter_get_basic(&iter,&(hnbInfo->HnbLocationInfo.gographicalLocation->AltitudeAndDirection.DirectionOfAltitude));
    		dbus_message_iter_next(&iter);	
    		dbus_message_iter_get_basic(&iter,&(hnbInfo->HnbLocationInfo.gographicalLocation->AltitudeAndDirection.altitude));
    	}
		dbus_message_iter_next(&iter);	
		unsigned int imsi_allowed_num = 0;
		dbus_message_iter_get_basic(&iter,&imsi_allowed_num);
		
		if(imsi_allowed_num)
		{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_recurse(&iter,&iter_array);
			IMSIWHITELIST* node = NULL;
			while(imsi_allowed_num)
			{
				node = (IMSIWHITELIST*)malloc(sizeof(IMSIWHITELIST));
				memset(node, 0, sizeof(IMSIWHITELIST));
				if(head)
				{
					head->next = node;
					head = head->next;
				}
				else
				{
					hnbInfo->imsi_white_list = node;
					head = node;
				}
				DBusMessageIter iter_struct;
				dbus_message_iter_recurse(&iter_array,&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(node->imsi[0]));
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(node->imsi[1]));
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(node->imsi[2]));
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(node->imsi[3]));
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(node->imsi[4]));
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(node->imsi[5]));
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(node->imsi[6]));
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(node->imsi[7]));
				dbus_message_iter_next(&iter_array);
				node = NULL;
				imsi_allowed_num--;
			}
		}
		dbus_message_iter_next(&iter);
	}

    dbus_message_unref(reply);
    return hnbInfo;
}


/*****************************************************
** DISCRIPTION:
**          get hnb information of all hnbs from iuhd
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

HNBLIST * mapi_get_hnb_list(int index, int localid, DBusConnection *dbus_connection, char *DBUS_METHOD)
{
    DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter  iter_array;
	DBusError err;
    HNBLIST *hnblist = NULL;
    int num,i;
    char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitFemtoDbusPath(localid,index,IUH_DBUS_BUSNAME,BUSNAME);
	ReInitFemtoDbusPath(localid,index,IUH_DBUS_OBJPATH,OBJPATH);
	ReInitFemtoDbusPath(localid,index,IUH_DBUS_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
	
	dbus_error_init(&err);
							 
	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);

    if (NULL == reply){
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)){
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}

    dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&num);
    
    hnblist = (HNBLIST *)malloc(sizeof(HNBLIST));
    hnblist->HNB_LIST = (HNBLIST *)malloc(sizeof(HNBLIST*));

    
    dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array);
	
    for(i = 0; i < num; i++){
        DBusMessageIter iter_struct;
		dbus_message_iter_recurse(&iter_array,&iter_struct);
		
        hnblist->HNB_LIST[i] = (struct hnb_info *)malloc(sizeof(struct hnb_info));
        memset(hnblist->HNB_LIST[i], 0, sizeof(struct hnb_info));

		dbus_message_iter_get_basic(&iter_struct,&(hnblist->HNB_LIST[i]->hnb_id));
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(hnblist->HNB_LIST[i]->hnb_name));
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(hnblist->HNB_LIST[i]->hnb_ip));
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(hnblist->HNB_LIST[i]->hnb_state));
        dbus_message_iter_next(&iter_array);  
        
    }
    hnblist->hnbcount = num;

    dbus_message_unref(reply);

    return hnblist;
}


/*****************************************************
** DISCRIPTION:
**          delete ue information by ue id
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

int mapi_delete_ue_id(int index, int localid, unsigned int ueid, DBusConnection *dbus_connection, char *DBUS_METHOD)
{
    DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int ret = 0;
	
    char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitFemtoDbusPath(localid,index,IUH_DBUS_BUSNAME,BUSNAME);
	ReInitFemtoDbusPath(localid,index,IUH_DBUS_OBJPATH,OBJPATH);
	ReInitFemtoDbusPath(localid,index,IUH_DBUS_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ueid,
							 DBUS_TYPE_INVALID);
    
	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply){
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)){
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_unref(reply);

	return ret;

}


/*****************************************************
** DISCRIPTION:
**          get ue information from iuhd by ue id 
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

Iuh_HNB_UE * mapi_get_ue_info_by_ueid(int index, int localid, unsigned int ue_id, int *ret, DBusConnection *dbus_connection, char *DBUS_METHOD)
{
    DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	Iuh_HNB_UE *ueInfo = NULL;
	char *identityValue;
	int i = 0;
	
    char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitFemtoDbusPath(localid,index,IUH_DBUS_BUSNAME,BUSNAME);
	ReInitFemtoDbusPath(localid,index,IUH_DBUS_OBJPATH,OBJPATH);
	ReInitFemtoDbusPath(localid,index,IUH_DBUS_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ue_id,
							 DBUS_TYPE_INVALID);
							 
	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply){
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)){
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}

    dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(*ret == 0){
	    ueInfo = (Iuh_HNB_UE *)malloc(sizeof(Iuh_HNB_UE));
    	memset(ueInfo, 0, sizeof(Iuh_HNB_UE));
    	dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ueInfo->UEID));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ueInfo->HNBID));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ueInfo->registrationCause));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ueInfo->context_id));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.present));
		switch(ueInfo->UE_Identity.present){
            case UE_Identity_iMSI: {
    	        dbus_message_iter_next(&iter);	
		        dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.imsi[0]));
		        dbus_message_iter_next(&iter);	
		        dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.imsi[1]));
		        dbus_message_iter_next(&iter);	
		        dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.imsi[2]));
		        dbus_message_iter_next(&iter);	
		        dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.imsi[3]));
		        dbus_message_iter_next(&iter);	
		        dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.imsi[4]));
		        dbus_message_iter_next(&iter);	
		        dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.imsi[5]));
		        dbus_message_iter_next(&iter);	
		        dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.imsi[6]));
		        dbus_message_iter_next(&iter);	
		        dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.imsi[7]));
    	        break;
    	    }
    	    case UE_Identity_tMSILAI: {
    	        dbus_message_iter_next(&iter);	
        		dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.tmsilai.tmsi[0]));
        		dbus_message_iter_next(&iter);	
        		dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.tmsilai.tmsi[1]));
        		dbus_message_iter_next(&iter);	
        		dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.tmsilai.tmsi[2]));
        		dbus_message_iter_next(&iter);	
        		dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.tmsilai.tmsi[3]));
        		dbus_message_iter_next(&iter);	
        		dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.tmsilai.tmsi[4]));
        		dbus_message_iter_next(&iter);	
        		dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.tmsilai.tmsi[5]));
        		dbus_message_iter_next(&iter);	
        		dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.tmsilai.tmsi[6]));
        		dbus_message_iter_next(&iter);	
        		dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.tmsilai.tmsi[7]));
        		dbus_message_iter_next(&iter);	
        		dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.tmsilai.lai.plmnid[0]));
        		dbus_message_iter_next(&iter);	
        		dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.tmsilai.lai.plmnid[1]));
        		dbus_message_iter_next(&iter);	
        		dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.tmsilai.lai.plmnid[2]));
        		dbus_message_iter_next(&iter);	
        		dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.tmsilai.lai.lac[0]));
        		dbus_message_iter_next(&iter);	
        		dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.tmsilai.lai.lac[1]));
    	        break;
    	    }
    	    case UE_Identity_pTMSIRAI: {
    	        dbus_message_iter_next(&iter);	
        		dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.ptmsirai.ptmsi[0]));
        		dbus_message_iter_next(&iter);	
        		dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.ptmsirai.ptmsi[1]));
        		dbus_message_iter_next(&iter);	
        		dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.ptmsirai.ptmsi[2]));
        		dbus_message_iter_next(&iter);	
        		dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.ptmsirai.ptmsi[3]));
        		dbus_message_iter_next(&iter);	
        		dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.ptmsirai.ptmsi[4]));
        		dbus_message_iter_next(&iter);	
        		dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.ptmsirai.ptmsi[5]));
        		dbus_message_iter_next(&iter);	
        		dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.ptmsirai.ptmsi[6]));
        		dbus_message_iter_next(&iter);	
        		dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.ptmsirai.ptmsi[7]));
        		dbus_message_iter_next(&iter);	
        		dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.ptmsirai.rai.rac[0]));
        		dbus_message_iter_next(&iter);	
        		dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.ptmsirai.rai.lai.plmnid[0]));
        		dbus_message_iter_next(&iter);	
        		dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.ptmsirai.rai.lai.plmnid[1]));
        		dbus_message_iter_next(&iter);	
        		dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.ptmsirai.rai.lai.plmnid[2]));
        		dbus_message_iter_next(&iter);	
        		dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.ptmsirai.rai.lai.lac[0]));
        		dbus_message_iter_next(&iter);	
        		dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.ptmsirai.rai.lai.lac[1]));
    	        break;
    	    }
    	    case UE_Identity_iMEI: {
    	        dbus_message_iter_next(&iter);	
		        dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.imei[0]));
		        dbus_message_iter_next(&iter);	
		        dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.imei[1]));
		        dbus_message_iter_next(&iter);	
		        dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.imei[2]));
		        dbus_message_iter_next(&iter);	
		        dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.imei[3]));
		        dbus_message_iter_next(&iter);	
		        dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.imei[4]));
		        dbus_message_iter_next(&iter);	
		        dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.imei[5]));
		        dbus_message_iter_next(&iter);	
		        dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.imei[6]));
		        dbus_message_iter_next(&iter);	
		        dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.imei[7]));
    	        break;
    	    }
    	    case UE_Identity_eSN: {
    	        dbus_message_iter_next(&iter);	
		        dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.esn[0]));
		        dbus_message_iter_next(&iter);	
		        dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.esn[1]));
		        dbus_message_iter_next(&iter);	
		        dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.esn[2]));
		        dbus_message_iter_next(&iter);	
		        dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.esn[3]));
    	        break;
    	    }
    	    case UE_Identity_iMSIDS41: {
    	        dbus_message_iter_next(&iter);	
		        dbus_message_iter_get_basic(&iter,&(identityValue));
		        ueInfo->UE_Identity.choice.imsids41 = malloc(strlen(identityValue)+1);
		        memset(ueInfo->UE_Identity.choice.imsids41,0,strlen(identityValue)+1);
		        memcpy(ueInfo->UE_Identity.choice.imsids41,identityValue,strlen(identityValue));
    	        break;
    	    }
    	    case UE_Identity_iMSIESN: {
    	        dbus_message_iter_next(&iter);	
        		dbus_message_iter_get_basic(&iter,&(identityValue));
        		ueInfo->UE_Identity.choice.imsiesn.imsids41 = malloc(strlen(identityValue)+1);
		        memset(ueInfo->UE_Identity.choice.imsiesn.imsids41,0,strlen(identityValue)+1);
		        memcpy(ueInfo->UE_Identity.choice.imsiesn.imsids41,identityValue,strlen(identityValue));
        		dbus_message_iter_next(&iter);	
        		dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.imsiesn.esn[0]));
        		dbus_message_iter_next(&iter);	
        		dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.imsiesn.esn[1]));
        		dbus_message_iter_next(&iter);	
        		dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.imsiesn.esn[2]));
        		dbus_message_iter_next(&iter);	
        		dbus_message_iter_get_basic(&iter,&(ueInfo->UE_Identity.choice.imsiesn.esn[3]));
    	        break;
    	    }
    	    case UE_Identity_tMSIDS41: {
    	        dbus_message_iter_next(&iter);	
		        dbus_message_iter_get_basic(&iter,&(identityValue));
		        ueInfo->UE_Identity.choice.tmsids41 = malloc(strlen(identityValue)+1);
		        memset(ueInfo->UE_Identity.choice.tmsids41,0,strlen(identityValue)+1);
		        memcpy(ueInfo->UE_Identity.choice.tmsids41,identityValue,strlen(identityValue));
    	        break;
    	    }
    	    default:
    	        break;
        }
        
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ueInfo->csgMemStatus));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ueInfo->state));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ueInfo->Capabilities.accStrRelIndicator));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ueInfo->Capabilities.csgIndicator));

	#ifdef RAB_INFO
		/* get ue rab information */
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ueInfo->rab_count));
        for(i = 0; i < ueInfo->rab_count; i++){
            char *trans_addr = NULL;
            Iu_RAB *ue_rab = NULL;
            ue_rab = (Iu_RAB*)malloc(sizeof(Iu_RAB));
            memset(ue_rab, 0, sizeof(Iu_RAB));
            dbus_message_iter_next(&iter);	
		    dbus_message_iter_get_basic(&iter,&(ue_rab->RABID[0]));    
		    dbus_message_iter_next(&iter);	
		    dbus_message_iter_get_basic(&iter,&(ue_rab->nAS_ScrIndicator[0]));
		    
		    dbus_message_iter_next(&iter);	
		    dbus_message_iter_get_basic(&iter,&(ue_rab->rab_para_list.deliveryOrder));
		    dbus_message_iter_next(&iter);	
		    dbus_message_iter_get_basic(&iter,&(ue_rab->rab_para_list.maxSDUSize));
		    dbus_message_iter_next(&iter);	
		    dbus_message_iter_get_basic(&iter,&(ue_rab->rab_para_list.rab_ass_indicator));
		    dbus_message_iter_next(&iter);	
		    dbus_message_iter_get_basic(&iter,&(ue_rab->rab_para_list.traffic_class));
		    
		    dbus_message_iter_next(&iter);	
		    dbus_message_iter_get_basic(&iter,&(ue_rab->user_plane_info.user_plane_mode));
		    dbus_message_iter_next(&iter);	
		    dbus_message_iter_get_basic(&iter,&(ue_rab->user_plane_info.up_modeVersions[0]));
		    dbus_message_iter_next(&iter);	
		    dbus_message_iter_get_basic(&iter,&(ue_rab->user_plane_info.up_modeVersions[1]));
		    
		    dbus_message_iter_next(&iter);	
		    dbus_message_iter_get_basic(&iter,&(trans_addr));
		    memcpy(ue_rab->trans_layer_info.transport_layer_addr, trans_addr, TRANSPORT_LAYER_ADDR_LEN);
		    dbus_message_iter_next(&iter);	
		    dbus_message_iter_get_basic(&iter,&(ue_rab->trans_layer_info.iu_trans_assoc.present));
		    if(ue_rab->trans_layer_info.iu_trans_assoc.present == IuTransportAssociation_gTP_TEI){
			    dbus_message_iter_next(&iter);	
    		    dbus_message_iter_get_basic(&iter,&(ue_rab->trans_layer_info.iu_trans_assoc.choice.gtp_tei[0]));
    		    dbus_message_iter_next(&iter);	
    		    dbus_message_iter_get_basic(&iter,&(ue_rab->trans_layer_info.iu_trans_assoc.choice.gtp_tei[1]));
    		    dbus_message_iter_next(&iter);	
    		    dbus_message_iter_get_basic(&iter,&(ue_rab->trans_layer_info.iu_trans_assoc.choice.gtp_tei[2]));
    		    dbus_message_iter_next(&iter);	
    		    dbus_message_iter_get_basic(&iter,&(ue_rab->trans_layer_info.iu_trans_assoc.choice.gtp_tei[3]));
			}
			else if(ue_rab->trans_layer_info.iu_trans_assoc.present == IuTransportAssociation_bindingID){
			    dbus_message_iter_next(&iter);	
    		    dbus_message_iter_get_basic(&iter,&(ue_rab->trans_layer_info.iu_trans_assoc.choice.binding_id[0]));
    		    dbus_message_iter_next(&iter);	
    		    dbus_message_iter_get_basic(&iter,&(ue_rab->trans_layer_info.iu_trans_assoc.choice.binding_id[1]));
    		    dbus_message_iter_next(&iter);	
    		    dbus_message_iter_get_basic(&iter,&(ue_rab->trans_layer_info.iu_trans_assoc.choice.binding_id[2]));
    		    dbus_message_iter_next(&iter);	
    		    dbus_message_iter_get_basic(&iter,&(ue_rab->trans_layer_info.iu_trans_assoc.choice.binding_id[3]));
			}
			
		    dbus_message_iter_next(&iter);	
		    dbus_message_iter_get_basic(&iter,&(ue_rab->service_handover));
		    dbus_message_iter_next(&iter);	
		    dbus_message_iter_get_basic(&iter,&(ue_rab->isPsDomain));
		    
		    dbus_message_iter_next(&iter);	
		    dbus_message_iter_get_basic(&iter,&(ue_rab->data_vol_rpt));
		    dbus_message_iter_next(&iter);	
		    dbus_message_iter_get_basic(&iter,&(ue_rab->dl_GTP_PDU_SeqNum));
		    dbus_message_iter_next(&iter);	
		    dbus_message_iter_get_basic(&iter,&(ue_rab->ul_GTP_PDU_seqNum));
		    dbus_message_iter_next(&iter);	
		    dbus_message_iter_get_basic(&iter,&(ue_rab->dl_N_PDU_SeqNum));
		    dbus_message_iter_next(&iter);	
		    dbus_message_iter_get_basic(&iter,&(ue_rab->ul_N_PDU_SeqNum));

		    ue_rab->rab_next = ueInfo->UE_RAB;
		    ueInfo->UE_RAB = ue_rab;
		    
        }
	#endif
	}
    dbus_message_unref(reply);
    
    return ueInfo;
}


/*****************************************************
** DISCRIPTION:
**          get information of all ues from iuhd
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

UELIST * mapi_get_ue_list(int index, int localid, DBusConnection *dbus_connection, char *DBUS_METHOD)
{
    DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter  iter_array;
	DBusError err;
    UELIST *uelist = NULL;
    int num,i;
    char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitFemtoDbusPath(localid,index,IUH_DBUS_BUSNAME,BUSNAME);
	ReInitFemtoDbusPath(localid,index,IUH_DBUS_OBJPATH,OBJPATH);
	ReInitFemtoDbusPath(localid,index,IUH_DBUS_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
	
	dbus_error_init(&err);
							 
	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);

    if (NULL == reply){
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)){
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}

    dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&num);

    //printf("recv num = %d\n",num);
    
    uelist = (UELIST *)malloc(sizeof(UELIST));
    uelist->UE_LIST = (UELIST *)malloc(sizeof(UELIST*));

    //printf("aaaaa\n");
    
    dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array);
	
    for(i = 0; i < num; i++){
        DBusMessageIter iter_struct;
		dbus_message_iter_recurse(&iter_array,&iter_struct);
		
        uelist->UE_LIST[i] = (struct ue_info *)malloc(sizeof(struct ue_info));
        memset(uelist->UE_LIST[i], 0, sizeof(struct ue_info));

		dbus_message_iter_get_basic(&iter_struct,&(uelist->UE_LIST[i]->ue_id));
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(uelist->UE_LIST[i]->hnb_id));
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(uelist->UE_LIST[i]->ue_state));
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(uelist->UE_LIST[i]->reg_cause));
        dbus_message_iter_next(&iter_array);
        //printf(" hnblist->HNB_LIST[%d]->hnb_id = %d\n",i,hnblist->HNB_LIST[i]->hnb_id);     
        
    }
    uelist->uecount = num;

    dbus_message_unref(reply);

    return uelist;
}
int mapi_femto_acl_white_list(int index, int localid, unsigned int op_type, unsigned int hnb_id, unsigned char* imsi, DBusConnection *dbus_connection, char *DBUS_METHOD)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	int ret = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitFemtoDbusPath(localid,index,IUH_DBUS_BUSNAME,BUSNAME);
	ReInitFemtoDbusPath(localid,index,IUH_DBUS_OBJPATH,OBJPATH);
	ReInitFemtoDbusPath(localid,index,IUH_DBUS_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&op_type,
                             DBUS_TYPE_UINT32,&hnb_id,
                             DBUS_TYPE_STRING,&imsi,
                             DBUS_TYPE_INVALID);					 
	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);

	if (NULL == reply){
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)){
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_unref(reply);

	return ret;
}
int mapi_femto_service_state_check(unsigned int insid, int islocal, int slot_id, unsigned int service_type, DBusConnection *dbus_connection, char *DBUS_METHOD)
{
    DBusMessage *query, *reply; 
    DBusMessageIter  iter;
    DBusError err;
    int ret = 0;
        
    query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,HMD_DBUS_INTERFACE,DBUS_METHOD);
    
    dbus_error_init(&err);

    dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&service_type,
                             DBUS_TYPE_UINT32,&slot_id,
                             DBUS_TYPE_UINT32,&insid,
                             DBUS_TYPE_UINT32,&islocal,
                             DBUS_TYPE_INVALID);
    
    reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
    
    dbus_message_unref(query);
    
    if (NULL == reply) {
        printf("<error> failed get reply.\n");
        if (dbus_error_is_set(&err)) {
            printf("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return -1;
    }
    
    dbus_message_iter_init(reply,&iter);
    dbus_message_iter_get_basic(&iter,&ret);

    dbus_message_unref(reply);

    return ret;

}

int mapi_femto_service_switch(unsigned int index, int islocal, int slot_id, unsigned int service_type, unsigned int service_switch, DBusConnection *dbus_connection, char *DBUS_METHOD)
{
    DBusMessage *query, *reply; 
    DBusMessageIter  iter;
    DBusError err;
    int ret = 0;
        
    query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,HMD_DBUS_INTERFACE,DBUS_METHOD);
    
    dbus_error_init(&err);

    dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&service_type,
                             DBUS_TYPE_UINT32,&service_switch,
                             DBUS_TYPE_UINT32,&index,
                             DBUS_TYPE_UINT32,&islocal,
                             DBUS_TYPE_UINT32,&slot_id,
                             DBUS_TYPE_INVALID);
    
    reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
    
    dbus_message_unref(query);
    
    if (NULL == reply) {
        printf("<error> failed get reply.\n");
        if (dbus_error_is_set(&err)) {
            printf("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return -1;
    }
    
    dbus_message_iter_init(reply,&iter);
    dbus_message_iter_get_basic(&iter,&ret);

    dbus_message_unref(reply);

    return ret;

}

/**********************************************************************************
 *  iuh_show_running_cfg_lib
 *	DESCRIPTION:
 * 		show and save iuh configration
 *	INPUT:		
 *		show string		
 *	OUTPUT:
 *          void
 * 	RETURN:
 *		0 		->	success
 *		other	->	fail
 *		
 **********************************************************************************/
#if 0
int mapi_get_running_cfg_lib(char *showStr, int index, DBusConnection *dbus_connection, char *DBUS_METHOD)
{	
    DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	char *tmp_str = NULL;
	
    char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitFemtoDbusPath(localid,index,IUH_DBUS_BUSNAME,BUSNAME);
	ReInitFemtoDbusPath(localid,index,IUH_DBUS_OBJPATH,OBJPATH);
	ReInitFemtoDbusPath(localid,index,IUH_DBUS_INTERFACE,INTERFACE);
	//printf("aaaaa\n");
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
	//printf("bbbbb\n");
	dbus_error_init(&err);
							 
	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);

    if (NULL == reply){
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)){
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&tmp_str);

	dbus_message_unref(reply);

	strncpy(showStr, tmp_str, strlen(tmp_str));
	
	return 0;	
}
#endif


