#ifdef _D_WCPSS_

#include <string.h>
#include <stdlib.h>
#include <dbus/dbus.h>
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#include "dbus/asd/ASDDbusDef1.h"


#include "asd_ac_roaming.h"


void dcli_free_one_node(struct Dcli_Info_Inter_AC_R_Group_T *inter_ac_r_group_t)
{
	if(inter_ac_r_group_t == NULL)
		return;
	if(inter_ac_r_group_t->ESSID != NULL) {
		free(inter_ac_r_group_t->ESSID);
		inter_ac_r_group_t->ESSID = NULL;
	}
	if(inter_ac_r_group_t->name != NULL) {
		free(inter_ac_r_group_t->name);
		inter_ac_r_group_t->name = NULL;
	}
	free(inter_ac_r_group_t);
	inter_ac_r_group_t = NULL;
	return;
	
}

void dcli_free_ac_roaming_info(struct Dcli_Info_Inter_AC_R_Group_T *ac)
{
	struct Dcli_Info_Inter_AC_R_Group_T *tmp = NULL;

	if(ac == NULL)
		return ;
	
	while(ac->next != NULL) {
		tmp = ac->next;
		ac->next = tmp->next;
		tmp->next = NULL;
		dcli_free_one_node(tmp);
	}
	free(ac);
	ac = NULL;
	return ;
	
}


/*nl add  20100113*/
Inter_AC_R_Group_T * show_ac_mobility_group(int localid,DBusConnection *dcli_dbus_connection, int index, unsigned char ID,unsigned int *num,unsigned int *ret)
{
	int i = 0;
	Inter_AC_R_Group_T *ACGROUP;	
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
			
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	unsigned char *ESSID = NULL;
	unsigned char *name = NULL;

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_AC_GROUP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_AC_GROUP_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_AC_GROUP_METHOD_SHOW_AC_GROUP);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&ID,
							 DBUS_TYPE_INVALID);

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

	if(*ret == 0 )
	{	
		ACGROUP = (Inter_AC_R_Group_T *)malloc(sizeof(Inter_AC_R_Group_T));
		memset(ACGROUP,0,sizeof(*ACGROUP));
		
		//ACGROUP->Mobility_AC= NULL;

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ACGROUP->GroupID));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ACGROUP->ESSID));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ACGROUP->name));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ACGROUP->host_ip));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,num);
		
		ACGROUP->Mobility_AC[0] = (Mobility_AC_Info_T *)malloc(sizeof(struct Mobility_AC_Info_T *));
		memset(ACGROUP->Mobility_AC[0], 0, sizeof(struct Mobility_AC_Info_T*));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		for (i = 0; i < *num; i++) {
			DBusMessageIter iter_struct;
			
			ACGROUP->Mobility_AC[i] = (Mobility_AC_Info_T *)malloc(sizeof(Mobility_AC_Info_T));			
			memset(ACGROUP->Mobility_AC[i], 0, sizeof(struct Mobility_AC_Info_T*));
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(ACGROUP->Mobility_AC[i]->ACID));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(ACGROUP->Mobility_AC[i]->ACIP));
		
			dbus_message_iter_next(&iter_array);
		}

		if((ESSID != NULL) && ((ACGROUP->ESSID= (unsigned char *)malloc(strlen(ESSID)+1)) != NULL)) {
			memset(ACGROUP->ESSID,0,strlen(ESSID)+1);
			memcpy(ACGROUP->ESSID,ESSID,strlen(ESSID));		
		}
		if((name != NULL) && ((ACGROUP->name= (unsigned char *)malloc(strlen(name)+1)) != NULL)) {
			memset(ACGROUP->name,0,strlen(name)+1);
			memcpy(ACGROUP->name,name,strlen(name));		
		}
	}
		
	dbus_message_unref(reply);
	
	return  ACGROUP;
}

//nl add 20100114 
Inter_AC_R_Group_T ** show_ac_mobility_list(int localid,DBusConnection *dcli_dbus_connection, int index, int *num,unsigned int *ret)
	{
	
		DBusMessage *query, *reply; 
		DBusMessageIter  iter;
		DBusMessageIter  iter_array;
		DBusMessageIter  iter_struct;
		DBusError	err;
		int i = 0;

		
		Inter_AC_R_Group_T **ACGROUP ;
		ACGROUP = (Inter_AC_R_Group_T**)malloc((*num)*sizeof(Inter_AC_R_Group_T*));
		
		char BUSNAME[PATH_LEN];
		char OBJPATH[PATH_LEN];
		char INTERFACE[PATH_LEN];
		
		ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
		ReInitDbusPath_V2(localid,index,ASD_DBUS_AC_GROUP_OBJPATH,OBJPATH);
		ReInitDbusPath_V2(localid,index,ASD_DBUS_AC_GROUP_INTERFACE,INTERFACE);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_AC_GROUP_METHOD_SHOW_AC_GROUP_LIST);
	
		dbus_error_init(&err);
	
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
		dbus_message_unref(query);
				
		if (NULL == reply) {
			*ret = ASD_DBUS_ERROR;
			if (dbus_error_is_set(&err)) {
				dbus_error_free(&err);
			}				
			return NULL;
		}
		
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,ret);
			
		if(*ret == 0 )
		{	
			unsigned char *ESSID = NULL;
			unsigned char *name = NULL;
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,num);
					
			dbus_message_iter_next(&iter);	
			dbus_message_iter_recurse(&iter,&iter_array);
			
			for (i = 0; i < *num; i++) {
			ACGROUP[i] = (Inter_AC_R_Group_T *)malloc(sizeof(Inter_AC_R_Group_T));
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(ACGROUP[i]->GroupID));
		
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(ESSID));

			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(name));

			dbus_message_iter_next(&iter_array);

			if((ESSID != NULL) && ((ACGROUP[i]->ESSID= (unsigned char *)malloc(strlen(ESSID)+1)) != NULL)) {
			memset(ACGROUP[i]->ESSID,0,strlen(ESSID)+1);
			memcpy(ACGROUP[i]->ESSID,ESSID,strlen(ESSID));		
		}
		if((name != NULL) && ((ACGROUP[i]->name= (unsigned char *)malloc(strlen(name)+1)) != NULL)) {
			memset(ACGROUP[i]->name,0,strlen(name)+1);
			memcpy(ACGROUP[i]->name,name,strlen(name));		
		}
												
			}
						
		}
			
		dbus_message_unref(reply);
		
		return ACGROUP;
	}




#endif

