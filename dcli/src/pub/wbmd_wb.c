#include <string.h>
#include <dbus/dbus.h>
#include <sys/stat.h>
#include "wbmd/wbmdpub.h"
#include "dbus/wbmd/WbmdDbusPath.h"
#include "dbus/wbmd/WbmdDbusDef.h"

void *dcli_show_wbridge_list(
	int index,
	unsigned int localid,
	unsigned int* ret,
	int wbid,
	DBusConnection *dcli_dbus_connection
	)
{	
	int num;
	int i = 0;	
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	dbus_error_init(&err);
	DCLI_WBRIDGE_API_GROUP * LIST = NULL;	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WBMD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WBMD_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WBMD_DBUS_INTERFACE,INTERFACE);	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WBMD_DBUS_CONF_METHOD_SHOW_WBRIDGE_LIST);
	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&wbid,								
						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);

	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		*ret = -1;
		return NULL;
	}

	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&(*ret));
	LIST = (DCLI_WBRIDGE_API_GROUP *)malloc(sizeof(DCLI_WBRIDGE_API_GROUP));
	memset(LIST, 0 , sizeof(DCLI_WBRIDGE_API_GROUP));
	if(*ret == 0 )
	{	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->wb_num);
	printf("%s LIST->wb_num %d\n",__func__,LIST->wb_num);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		LIST->wbridge = (struct wbridge_info*)malloc((LIST->wb_num)*sizeof(struct wbridge_info*));
		for (i = 0; i < LIST->wb_num; i++) {
			DBusMessageIter iter_struct;
			LIST->wbridge[i] = (struct wbridge_info*)malloc(sizeof(struct wbridge_info));
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(LIST->wbridge[i]->WBID));
		
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(LIST->wbridge[i]->IP));
		
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(LIST->wbridge[i]->WBState));

			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(LIST->wbridge[i]->access_time));

			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(LIST->wbridge[i]->last_access_time));

			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(LIST->wbridge[i]->leave_time));

			dbus_message_iter_next(&iter_array);
		}
	}
	dbus_message_unref(reply);
	printf("%s LIST->wb_num %d\n",__func__,LIST->wb_num);
	return LIST;

}

void *dcli_show_wbridge_basic_info(
	int index,
	unsigned int localid,
	unsigned int* ret,
	int wbid,
	DBusConnection *dcli_dbus_connection
	)
{	
	int num;
	int i = 0;	
	int j = 0;
	char *ifname = NULL;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	dbus_error_init(&err);
	DCLI_WBRIDGE_API_GROUP * LIST = NULL;	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WBMD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WBMD_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WBMD_DBUS_INTERFACE,INTERFACE);	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WBMD_DBUS_CONF_METHOD_SHOW_WBRIDGE_BASIC_INFO);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&wbid,								
						DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);

	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		*ret = -1;
		return NULL;
	}

	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&(*ret));
	LIST = (DCLI_WBRIDGE_API_GROUP *)malloc(sizeof(DCLI_WBRIDGE_API_GROUP));
	memset(LIST, 0 , sizeof(DCLI_WBRIDGE_API_GROUP));
	if(*ret == 0 )
	{	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->wb_num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		LIST->wbridge = (struct wbridge_info*)malloc((LIST->wb_num)*sizeof(struct wbridge_info*));
		for (i = 0; i < LIST->wb_num; i++) {
			DBusMessageIter iter_struct;			
 			DBusMessageIter iter_sub_array;
			DBusMessageIter iter_sub_struct;
			LIST->wbridge[i] = (struct wbridge_info*)malloc(sizeof(struct wbridge_info));
			dbus_message_iter_recurse(&iter_array,&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(LIST->wbridge[i]->WBID));

			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(LIST->wbridge[i]->IP));

			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(LIST->wbridge[i]->WBState));

			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(LIST->wbridge[i]->if_num));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
			for(j = 0; j < LIST->wbridge[i]->if_num; j++)
			{
				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBIF[j].ifIndex)); 

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&ifname);
				strcpy(LIST->wbridge[i]->WBIF[j].ifDescr, ifname);

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBIF[j].ifType));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBIF[j].ifMtu));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBIF[j].ifSpeed));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBIF[j].ifPhysAddress[0]));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBIF[j].ifPhysAddress[1]));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBIF[j].ifPhysAddress[2]));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBIF[j].ifPhysAddress[3]));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBIF[j].ifPhysAddress[4]));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBIF[j].ifPhysAddress[5]));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBIF[j].ifAdminStatus));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBIF[j].ifOperStatus));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBIF[j].ifLastChange));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBIF[j].ifInOctets));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBIF[j].ifInUcastPkts));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBIF[j].ifInNUcastPkts));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBIF[j].ifInDiscards));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBIF[j].ifInErrors));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBIF[j].ifInUnknownProtos));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBIF[j].ifOutOctets));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBIF[j].ifOutUcastPkts));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBIF[j].ifOutNUcastPkts));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBIF[j].ifOutDiscards));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBIF[j].ifOutErrors));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBIF[j].ifOutQLen));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBIF[j].ifSpecific));

				dbus_message_iter_next(&iter_sub_array);
			}
			dbus_message_iter_next(&iter_array);
		}
	}
	dbus_message_unref(reply);
	return LIST;

}


void *dcli_show_wbridge_mint_info(
	int index,
	unsigned int localid,
	unsigned int* ret,
	int wbid,
	DBusConnection *dcli_dbus_connection
	)
{	
	int num;
	int i = 0;	
	int j = 0;
	char *name = NULL;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	dbus_error_init(&err);
	DCLI_WBRIDGE_API_GROUP * LIST = NULL;	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WBMD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WBMD_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WBMD_DBUS_INTERFACE,INTERFACE);	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WBMD_DBUS_CONF_METHOD_SHOW_WBRIDGE_MINT_INFO);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&wbid,								
						DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);

	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		*ret = -1;
		return NULL;
	}

	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&(*ret));
	LIST = (DCLI_WBRIDGE_API_GROUP *)malloc(sizeof(DCLI_WBRIDGE_API_GROUP));
	memset(LIST, 0 , sizeof(DCLI_WBRIDGE_API_GROUP));
	if(*ret == 0 )
	{	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->wb_num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		LIST->wbridge = (struct wbridge_info*)malloc((LIST->wb_num)*sizeof(struct wbridge_info*));
		for (i = 0; i < LIST->wb_num; i++) {
			DBusMessageIter iter_struct;			
 			DBusMessageIter iter_sub_array;
			DBusMessageIter iter_sub_struct;
			LIST->wbridge[i] = (struct wbridge_info*)malloc(sizeof(struct wbridge_info));
			dbus_message_iter_recurse(&iter_array,&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(LIST->wbridge[i]->WBID));

			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(LIST->wbridge[i]->IP));

			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(LIST->wbridge[i]->WBState));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
			for(j = 0; j < 1; j++)
			{
				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);

				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBMintNode.netAddress[0]));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBMintNode.netAddress[1]));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBMintNode.netAddress[2]));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBMintNode.netAddress[3]));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBMintNode.netAddress[4]));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBMintNode.netAddress[5]));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBMintNode.nodeType));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBMintNode.nodeMode));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBMintNode.linksCount));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBMintNode.nodesCount));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBMintNode.nodeInterfaceId));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBMintNode.protocolEnabled));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&name);
				strcpy(LIST->wbridge[i]->WBMintNode.nodeName, name);

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBMintNode.autoBitrateEnable));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBMintNode.autoBitrateAddition));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBMintNode.autoBitrateMinLevel));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBMintNode.extraCost));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBMintNode.fixedCost));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBMintNode.nodeID));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBMintNode.ampLow));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBMintNode.ampHigh));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBMintNode.authMode));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBMintNode.authRelay));
				
				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBMintNode.crypt));
				
				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBMintNode.compress));
				
				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBMintNode.overTheAirUpgradeEnable));
				
				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBMintNode.overTheAirUpgradeSpeed));
				
				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBMintNode.roaming));
				
				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBMintNode.polling));
				
				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBMintNode.mintBroadcastRate));
				
				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBMintNode.noiseFloor));
				
				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&name);
				strcpy(LIST->wbridge[i]->WBMintNode.secretKey, name);

				dbus_message_iter_next(&iter_sub_array);
			}
			dbus_message_iter_next(&iter_array);
		}
	}
	dbus_message_unref(reply);
	return LIST;

}


void *dcli_show_wbridge_rf_info(
	int index,
	unsigned int localid,
	unsigned int* ret,
	int wbid,
	DBusConnection *dcli_dbus_connection
	)
{	
	int num;
	int i = 0;	
	int j = 0;
	char *name = NULL;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	dbus_error_init(&err);
	DCLI_WBRIDGE_API_GROUP * LIST = NULL;	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WBMD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WBMD_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WBMD_DBUS_INTERFACE,INTERFACE);	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WBMD_DBUS_CONF_METHOD_SHOW_WBRIDGE_RF_INFO);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&wbid,								
						DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);

	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		*ret = -1;
		return NULL;
	}

	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&(*ret));
	LIST = (DCLI_WBRIDGE_API_GROUP *)malloc(sizeof(DCLI_WBRIDGE_API_GROUP));
	memset(LIST, 0 , sizeof(DCLI_WBRIDGE_API_GROUP));
	if(*ret == 0 )
	{	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->wb_num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		LIST->wbridge = (struct wbridge_info*)malloc((LIST->wb_num)*sizeof(struct wbridge_info*));
		for (i = 0; i < LIST->wb_num; i++) {
			DBusMessageIter iter_struct;			
 			DBusMessageIter iter_sub_array;
			DBusMessageIter iter_sub_struct;
			LIST->wbridge[i] = (struct wbridge_info*)malloc(sizeof(struct wbridge_info));
			dbus_message_iter_recurse(&iter_array,&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(LIST->wbridge[i]->WBID));

			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(LIST->wbridge[i]->IP));

			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(LIST->wbridge[i]->WBState));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
			for(j = 0; j < 1; j++)
			{
				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);

				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBRmProperty.rmPropertiesIfIndex));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&name);
				strcpy(LIST->wbridge[i]->WBRmProperty.rmType, name);

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBRmProperty.rmFrequency));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBRmProperty.rmBitRate));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBRmProperty.rmSid));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBRmProperty.rmCurPowerLevel));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBRmProperty.rmModulation));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBRmProperty.rmAntenna));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBRmProperty.rmDistance));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBRmProperty.rmBurst));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBRmProperty.rmLongRange));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBRmProperty.rmPowerCtl));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBRmProperty.rmTXRT));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBRmProperty.rmTXVRT));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBRmProperty.rmPTP));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBRmProperty.rmWOCD));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBRmProperty.rmBCsid));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBRmProperty.rmDistanceAuto));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBRmProperty.rmNoiseFloor));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBRmProperty.rmBandwidth));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LIST->wbridge[i]->WBRmProperty.rmChainMode));
				
				dbus_message_iter_next(&iter_sub_array);
			}
			dbus_message_iter_next(&iter_array);
		}
	}
	dbus_message_unref(reply);
	return LIST;

}


void dcli_wbridge_list_free(DCLI_WBRIDGE_API_GROUP * LIST){
	int i = 0;
	if(LIST == NULL)
		return;
	for(i = 0; i < LIST->wb_num; i++){
		if(LIST->wbridge[i]){
			free(LIST->wbridge[i]);
			LIST->wbridge[i] = NULL;
		}
	}
	free(LIST);
	LIST = NULL;
	return;
}


