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
#include <dirent.h>
#include <stdio.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <syslog.h>
#include <dirent.h>
#include <unistd.h>
#include <assert.h>
#include "wbmd.h"
#include "wbmd/wbmdpub.h"
#include "dbus/wbmd/WbmdDbusDef.h"
#include "wbmd_thread.h"
#include "wbmd_dbus.h"
#include "wbmd_check.h"
#include "wbmd_manage.h"
#include "wbmd_dbushandle.h"


static DBusConnection * wbmd_dbus_connection = NULL;


DBusMessage * wbmd_dbus_interface_create_wbridge(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	int ID;
	DBusError err;
	int ret = WBMD_DBUS_SUCCESS;
	int IP = 0;
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&ID,
								DBUS_TYPE_UINT32,&IP,
								DBUS_TYPE_INVALID))){

		printf("Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(ID >= WBRIDGE_NUM){
		ret = WBMD_DBUS_NUM_OVER_MAX;
	}else if(wBridge[ID] != NULL){
		ret = WBMD_DBUS_ID_EXIST;
	}if(wbridge_get(IP) != NULL){
		ret = WBMD_DBUS_ID_EXIST;
	}else{
		ret = wbmd_wbridge_create(ID,IP);
	}
	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&ret,
							 DBUS_TYPE_INVALID);
	return reply;
	
}

DBusMessage * wbmd_dbus_interface_show_wbridge_list(DBusConnection *conn, DBusMessage *msg, void *user_data){
	
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int num=0;
	DBusError err;
	int ret = WBMD_DBUS_SUCCESS;
	dbus_error_init(&err);
	int i=0;
	struct wbridge_info *wb[WBRIDGE_NUM];
	int WBID;
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&WBID,
								DBUS_TYPE_INVALID))){

		printf("Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(WBID == 0){
		while(i<WBRIDGE_NUM){
			if(wBridge[i] != NULL)
			{
				wb[num] = wBridge[i];
				num++;
			}
			i++;
		}
	}else{
		if(wBridge[WBID] != NULL){
			wb[num] = wBridge[WBID];
			num++;
		}
	}
	if(num == 0)
		ret = WBMD_DBUS_ID_NO_EXIST;
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret);
	if(ret == WBMD_DBUS_SUCCESS){
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
						  &(wb[i]->WBID));
				
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
						  &(wb[i]->IP));
				
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
						  &(wb[i]->WBState));

			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
						  &(wb[i]->access_time));
				
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
						  &(wb[i]->last_access_time));
				
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
						  &(wb[i]->leave_time));
					
			dbus_message_iter_close_container (&iter_array, &iter_struct);


		}
					
		dbus_message_iter_close_container (&iter, &iter_array);
	}
	
	return reply;	
}

DBusMessage * wbmd_dbus_interface_wbridge(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	unsigned int WBID = 0;
	unsigned int IP = 0;
	DBusError err;
	int ret = WBMD_DBUS_SUCCESS;
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&WBID,
								DBUS_TYPE_INVALID))){

		printf("Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(wBridge[WBID] == NULL){
		ret = WBMD_DBUS_ID_NO_EXIST;
	}else if(wBridge[WBID]->WBState == 0){
		ret = WBMD_DBUS_NOT_RUNNING;
	}else{
		IP = wBridge[WBID]->IP;
	}
	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&ret,
							 DBUS_TYPE_UINT32,&IP,
							 DBUS_TYPE_INVALID);
	return reply;
	
}

DBusMessage * wbmd_dbus_interface_delete_wbridge(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	unsigned int WBID = 0;
	DBusError err;
	int ret = WBMD_DBUS_SUCCESS;
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&WBID,
								DBUS_TYPE_INVALID))){

		printf("Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(wBridge[WBID] == NULL){
		ret = WBMD_DBUS_ID_NO_EXIST;
	}else{
		ret = wbmd_wbridge_delete(WBID);
	}
	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&ret,
							 DBUS_TYPE_INVALID);
	return reply;
	
}



DBusMessage * wbmd_dbus_interface_set_wbridge_snmp(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	int ID;	
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	int ret = WBMD_DBUS_SUCCESS;
	int i = 0;
	int num = 0;
	char *argv_t[255];
	dbus_error_init(&err);

	dbus_message_iter_init(msg,&iter);
	dbus_message_iter_get_basic(&iter,&ID);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&num);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array);

	for (i = 1; i < num+1; i++) {
		DBusMessageIter iter_struct;

		dbus_message_iter_recurse(&iter_array,&iter_struct);
	
		dbus_message_iter_get_basic(&iter_struct,&(argv_t[i]));

		dbus_message_iter_next(&iter_array);
	}
	if(ID >= WBRIDGE_NUM){
		ret = WBMD_DBUS_NUM_OVER_MAX;
	}else if(wBridge[ID] == NULL){
		ret = WBMD_DBUS_ID_NO_EXIST;
	}else{
		ret = wbmd_wbridge_snmp_init(ID,argv_t,num);
	}
	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&ret,
							 DBUS_TYPE_INVALID);
	return reply;
	
}


DBusMessage * wbmd_dbus_interface_show_wbridge_basic_info(DBusConnection *conn, DBusMessage *msg, void *user_data){
	
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusMessageIter iter_sub_array;
	DBusMessageIter iter_sub_struct;
	unsigned int num=0;
	DBusError err;
	int ret = WBMD_DBUS_SUCCESS;
	dbus_error_init(&err);
	int i=0;
	int j = 0;
	int WBID;
	char *name = NULL;
	struct wbridge_info *wb[WBRIDGE_NUM];
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&WBID,
								DBUS_TYPE_INVALID))){

		printf("Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(WBID == 0){
		while(i<WBRIDGE_NUM){
			if(wBridge[i] != NULL)
			{
				wb[num] = wBridge[i];
				num++;
			}
			i++;
		}
	}else{
		if(wBridge[WBID] != NULL){
			wb[num] = wBridge[WBID];
			num++;
		}
	}
	if(num == 0)
		ret = WBMD_DBUS_ID_NO_EXIST;
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret);
	if(ret == WBMD_DBUS_SUCCESS){
		name = (char *)malloc(16);
		memset(name, 0, 16);
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
												DBUS_TYPE_ARRAY_AS_STRING
												DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_STRING_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
												DBUS_STRUCT_END_CHAR_AS_STRING
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
						  &(wb[i]->WBID));

			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
						  &(wb[i]->IP));
				
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
						  &(wb[i]->WBState));
				
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
						  &(wb[i]->if_num));
				
			dbus_message_iter_open_container (&iter_struct,
											DBUS_TYPE_ARRAY,
											DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_STRING_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
											DBUS_STRUCT_END_CHAR_AS_STRING,
										  &iter_sub_array);
			for(j = 0;j < wb[i]->if_num; j++)
		   	{
		       
				dbus_message_iter_open_container (&iter_sub_array,
				 									 DBUS_TYPE_STRUCT,
				 									 NULL,
													 &iter_sub_struct);
						
				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBIF[j].ifIndex));

				memset(name, 0, 16);
				strcpy(name, wb[i]->WBIF[j].ifDescr);
				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_STRING ,&name);

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBIF[j].ifType));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBIF[j].ifMtu));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBIF[j].ifSpeed));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_BYTE ,&(wb[i]->WBIF[j].ifPhysAddress[0]));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_BYTE ,&(wb[i]->WBIF[j].ifPhysAddress[1]));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_BYTE ,&(wb[i]->WBIF[j].ifPhysAddress[2]));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_BYTE ,&(wb[i]->WBIF[j].ifPhysAddress[3]));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_BYTE ,&(wb[i]->WBIF[j].ifPhysAddress[4]));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_BYTE ,&(wb[i]->WBIF[j].ifPhysAddress[5]));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBIF[j].ifAdminStatus));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBIF[j].ifOperStatus));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBIF[j].ifLastChange));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBIF[j].ifInOctets));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBIF[j].ifInUcastPkts));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBIF[j].ifInNUcastPkts));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBIF[j].ifInDiscards));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBIF[j].ifInErrors));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBIF[j].ifInUnknownProtos));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBIF[j].ifOutOctets));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBIF[j].ifOutUcastPkts));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBIF[j].ifOutNUcastPkts));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBIF[j].ifOutDiscards));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBIF[j].ifOutErrors));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBIF[j].ifOutQLen));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBIF[j].ifSpecific));

		    	dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);
			}
			dbus_message_iter_close_container (&iter_struct, &iter_sub_array);
			dbus_message_iter_close_container (&iter_array, &iter_struct);
		}
		dbus_message_iter_close_container (&iter, &iter_array);
		if(name)
			free(name);
	}
	
	return reply;	
}



DBusMessage * wbmd_dbus_interface_show_wbridge_mint_info(DBusConnection *conn, DBusMessage *msg, void *user_data){
	
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusMessageIter iter_sub_array;
	DBusMessageIter iter_sub_struct;
	unsigned int num=0;
	DBusError err;
	int ret = WBMD_DBUS_SUCCESS;
	dbus_error_init(&err);
	int i=0;
	int j = 0;
	int WBID;
	char *name = NULL;
	struct wbridge_info *wb[WBRIDGE_NUM];
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&WBID,
								DBUS_TYPE_INVALID))){

		printf("Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(WBID == 0){
		while(i<WBRIDGE_NUM){
			if(wBridge[i] != NULL)
			{
				wb[num] = wBridge[i];
				num++;
			}
			i++;
		}
	}else{
		if(wBridge[WBID] != NULL){
			wb[num] = wBridge[WBID];
			num++;
		}
	}
	if(num == 0)
		ret = WBMD_DBUS_ID_NO_EXIST;
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret);
	if(ret == WBMD_DBUS_SUCCESS){
		name = (char *)malloc(DEFAULT_LEN);
		memset(name, 0, DEFAULT_LEN);
		dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &num);
			
		dbus_message_iter_open_container (&iter,
										DBUS_TYPE_ARRAY,
										DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING					
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_ARRAY_AS_STRING
												DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_STRING_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_STRING_AS_STRING
												DBUS_STRUCT_END_CHAR_AS_STRING
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
						  &(wb[i]->WBID));

			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
						  &(wb[i]->IP));
				
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
						  &(wb[i]->WBState));
							
			dbus_message_iter_open_container (&iter_struct,
											DBUS_TYPE_ARRAY,
											DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_STRING_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_STRING_AS_STRING
											DBUS_STRUCT_END_CHAR_AS_STRING,
										  &iter_sub_array);
			for(j = 0;j < 1; j++)
		   	{
		       
				dbus_message_iter_open_container (&iter_sub_array,
				 									 DBUS_TYPE_STRUCT,
				 									 NULL,
													 &iter_sub_struct);
						
				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_BYTE ,&(wb[i]->WBMintNode.netAddress[0]));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_BYTE ,&(wb[i]->WBMintNode.netAddress[1]));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_BYTE ,&(wb[i]->WBMintNode.netAddress[2]));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_BYTE ,&(wb[i]->WBMintNode.netAddress[3]));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_BYTE ,&(wb[i]->WBMintNode.netAddress[4]));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_BYTE ,&(wb[i]->WBMintNode.netAddress[5]));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBMintNode.nodeType));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBMintNode.nodeMode));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBMintNode.linksCount));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBMintNode.nodesCount));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBMintNode.nodeInterfaceId));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBMintNode.protocolEnabled));

				memset(name, 0, DEFAULT_LEN);
				strcpy(name, wb[i]->WBMintNode.nodeName);
				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_STRING ,&name);

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBMintNode.autoBitrateEnable));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBMintNode.autoBitrateAddition));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBMintNode.autoBitrateMinLevel));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBMintNode.extraCost));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBMintNode.fixedCost));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBMintNode.nodeID));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBMintNode.ampLow));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBMintNode.ampHigh));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBMintNode.authMode));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBMintNode.authRelay));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBMintNode.crypt));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBMintNode.compress));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBMintNode.overTheAirUpgradeEnable));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBMintNode.overTheAirUpgradeSpeed));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBMintNode.roaming));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBMintNode.polling));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBMintNode.mintBroadcastRate));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBMintNode.noiseFloor));

				memset(name, 0, DEFAULT_LEN);
				strcpy(name, wb[i]->WBMintNode.secretKey);
				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_STRING ,&name);

		    	dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);
			}
			dbus_message_iter_close_container (&iter_struct, &iter_sub_array);
			dbus_message_iter_close_container (&iter_array, &iter_struct);
		}
		dbus_message_iter_close_container (&iter, &iter_array);
		if(name)
			free(name);
	}
	
	return reply;	
}

DBusMessage * wbmd_dbus_interface_show_wbridge_rf_info(DBusConnection *conn, DBusMessage *msg, void *user_data){
	
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusMessageIter iter_sub_array;
	DBusMessageIter iter_sub_struct;
	unsigned int num=0;
	DBusError err;
	int ret = WBMD_DBUS_SUCCESS;
	dbus_error_init(&err);
	int i=0;
	int j = 0;
	int WBID;
	char *name = NULL;
	struct wbridge_info *wb[WBRIDGE_NUM];
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&WBID,
								DBUS_TYPE_INVALID))){

		printf("Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(WBID == 0){
		while(i<WBRIDGE_NUM){
			if(wBridge[i] != NULL)
			{
				wb[num] = wBridge[i];
				num++;
			}
			i++;
		}
	}else{
		if(wBridge[WBID] != NULL){
			wb[num] = wBridge[WBID];
			num++;
		}
	}
	if(num == 0)
		ret = WBMD_DBUS_ID_NO_EXIST;
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret);
	if(ret == WBMD_DBUS_SUCCESS){
		name = (char *)malloc(DEFAULT_LEN);
		memset(name, 0, DEFAULT_LEN);
		dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &num);
			
		dbus_message_iter_open_container (&iter,
										DBUS_TYPE_ARRAY,
										DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING					
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_ARRAY_AS_STRING
												DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_STRING_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
												DBUS_STRUCT_END_CHAR_AS_STRING
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
						  &(wb[i]->WBID));

			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
						  &(wb[i]->IP));
				
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
						  &(wb[i]->WBState));
							
			dbus_message_iter_open_container (&iter_struct,
											DBUS_TYPE_ARRAY,
											DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_STRING_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
											DBUS_STRUCT_END_CHAR_AS_STRING,
										  &iter_sub_array);
			for(j = 0;j < 1; j++)
		   	{
		       
				dbus_message_iter_open_container (&iter_sub_array,
				 									 DBUS_TYPE_STRUCT,
				 									 NULL,
													 &iter_sub_struct);

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBRmProperty.rmPropertiesIfIndex));

				memset(name, 0, DEFAULT_LEN);
				strcpy(name, wb[i]->WBRmProperty.rmType);
				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_STRING ,&name);

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBRmProperty.rmFrequency));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBRmProperty.rmBitRate));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBRmProperty.rmSid));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBRmProperty.rmCurPowerLevel));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBRmProperty.rmModulation));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBRmProperty.rmAntenna));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBRmProperty.rmDistance));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBRmProperty.rmBurst));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBRmProperty.rmLongRange));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBRmProperty.rmPowerCtl));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBRmProperty.rmTXRT));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBRmProperty.rmTXVRT));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBRmProperty.rmPTP));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBRmProperty.rmWOCD));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBRmProperty.rmBCsid));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBRmProperty.rmDistanceAuto));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBRmProperty.rmNoiseFloor));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBRmProperty.rmBandwidth));

				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_UINT32 ,&(wb[i]->WBRmProperty.rmChainMode));

		    	dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);
			}
			dbus_message_iter_close_container (&iter_struct, &iter_sub_array);
			dbus_message_iter_close_container (&iter_array, &iter_struct);
		}
		dbus_message_iter_close_container (&iter, &iter_array);
		if(name)
			free(name);
	}
	
	return reply;	
}


DBusMessage * wbmd_dbus_interface_show_running_config(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage* reply;
	DBusMessageIter  iter;
	unsigned int num=0;
	char *showStr = NULL,*cursor = NULL;
	int totalLen = 0;
	struct wbridge_info *wb[WBRIDGE_NUM];
	int i = 0;	
	int j = 0;
	while(i<WBRIDGE_NUM){
		if(wBridge[i] != NULL)
		{
			wb[num] = wBridge[i];
			num++;
		}
		i++;
	}
	if(num == 0){		
		showStr = (char*)malloc(1);		
		memset(showStr,0,1);
	}else{
		showStr = (char*)malloc(num*1024);
	
		if(NULL == showStr) {

		}else{
			memset(showStr,0,num*1024);
			cursor = showStr;			
			for(i=0; i<num; i++){								
				totalLen += sprintf(cursor,"create wbridge %d %s\n",wb[i]->WBID,inet_ntoa((wb[i]->wbaddr.sin_addr)));
				cursor = showStr + totalLen;
				if(wb[i]->argn != 0){
					printf("wb[i]->argn %d\n",wb[i]->argn);
					totalLen += sprintf(cursor,"set wbridge %d snmp ",wb[i]->WBID);
					cursor = showStr + totalLen;
					for(j = 1; j < wb[i]->argn-1; j++){
						totalLen += sprintf(cursor,"%s ",wb[i]->argv[j]);
						cursor = showStr + totalLen;
						printf("wb[i]->argv[%d] %s\n",j,wb[i]->argv[j]);
					}
					totalLen += sprintf(cursor,"\n");
					cursor = showStr + totalLen;
				}
			}
		}
	}
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_STRING,
									 &showStr);	
	free(showStr);
	showStr = NULL;
	return reply;
}



static DBusHandlerResult wbmd_dbus_message_handler (DBusConnection *connection, DBusMessage *message, void *user_data){

	DBusMessage		*reply = NULL;

	if(message == NULL)
		return DBUS_HANDLER_RESULT_HANDLED;

	if	(strcmp(dbus_message_get_path(message),WBMD_DBUS_OBJPATH) == 0)	{
		if (dbus_message_is_method_call(message,WBMD_DBUS_INTERFACE,WBMD_DBUS_CONF_METHOD_CREATE_WBRIDGE)) {
			reply = wbmd_dbus_interface_create_wbridge(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,WBMD_DBUS_INTERFACE,WBMD_DBUS_CONF_METHOD_SHOW_WBRIDGE_LIST)) {
			reply = wbmd_dbus_interface_show_wbridge_list(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,WBMD_DBUS_INTERFACE,WBMD_DBUS_CONF_METHOD_WBRIDGE)) {
			reply = wbmd_dbus_interface_wbridge(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,WBMD_DBUS_INTERFACE,WBMD_DBUS_CONF_METHOD_SET_WBRIDGE_SNMP)) {
			reply = wbmd_dbus_interface_set_wbridge_snmp(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,WBMD_DBUS_INTERFACE,WBMD_DBUS_CONF_METHOD_SHOW_WBRIDGE_BASIC_INFO)) {
			reply = wbmd_dbus_interface_show_wbridge_basic_info(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,WBMD_DBUS_INTERFACE,WBMD_DBUS_CONF_METHOD_SHOW_WBRIDGE_MINT_INFO)) {
			reply = wbmd_dbus_interface_show_wbridge_mint_info(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,WBMD_DBUS_INTERFACE,WBMD_DBUS_CONF_METHOD_SHOW_WBRIDGE_RF_INFO)) {
			reply = wbmd_dbus_interface_show_wbridge_rf_info(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,WBMD_DBUS_INTERFACE,WBMD_DBUS_CONF_METHOD_SHOW_WBRIDGE_RUNNING_CONFIG)) {
			reply = wbmd_dbus_interface_show_running_config(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,WBMD_DBUS_INTERFACE,WBMD_DBUS_CONF_METHOD_DELETE_WBRIDGE)) {
			reply = wbmd_dbus_interface_delete_wbridge(connection,message,user_data);
		}
	}
	if (reply) {
		dbus_connection_send (connection, reply, NULL);
		dbus_connection_flush(connection); 
		dbus_message_unref (reply);
	}

	return DBUS_HANDLER_RESULT_HANDLED ;
}

void *wbmd_dbus_thread_restart()
{
	wbmd_pid_write_v2("WBMDDbus Reinit");	
	if(wbmd_dbus_reinit()
	){
		while (dbus_connection_read_write_dispatch(wbmd_dbus_connection,500)) {

		}
	}
	wbmd_syslog_err("there is something wrong in dbus handler\n");	
	return 0;
}



DBusHandlerResult
wbmd_dbus_filter_function (DBusConnection * connection,
					   DBusMessage * message, void *user_data)
{
	if (dbus_message_is_signal (message, DBUS_INTERFACE_LOCAL, "Disconnected") &&
		   strcmp (dbus_message_get_path (message), DBUS_PATH_LOCAL) == 0) {

		wbmd_syslog_err("Got disconnected from the system message bus; "
				"retrying to reconnect every 3000 ms\n");
		dbus_connection_close(wbmd_dbus_connection);
		wbmd_dbus_connection = NULL;
		WbmdThread thread_dbus; 
		if(!(WbmdCreateThread(&thread_dbus, wbmd_dbus_thread_restart, NULL,0))) {
			wbmd_syslog_crit("Error starting Dbus Thread");
			exit(1);
		}

	} else if (dbus_message_is_signal (message,
			      DBUS_INTERFACE_DBUS,
			      "NameOwnerChanged")) {

	} else
		return TRUE;

	return DBUS_HANDLER_RESULT_HANDLED;
}

int wbmd_dbus_reinit(void)
{	
	int i = 0;
	DBusError dbus_error;
	dbus_threads_init_default();
	
	DBusObjectPathVTable	wbmd_vtable = {NULL, &wbmd_dbus_message_handler, NULL, NULL, NULL, NULL};	


	dbus_connection_set_change_sigpipe (TRUE);

	dbus_error_init (&dbus_error);
	wbmd_dbus_connection = dbus_bus_get_private (DBUS_BUS_SYSTEM, &dbus_error);
	if (wbmd_dbus_connection == NULL) {
		wbmd_syslog_err("dbus_bus_get(): %s\n", dbus_error.message);
		return FALSE;
	}

	// Use npd to handle subsection of NPD_DBUS_OBJPATH including slots
	if (!dbus_connection_register_fallback (wbmd_dbus_connection, WBMD_DBUS_OBJPATH, &wbmd_vtable, NULL)) {
		wbmd_syslog_err("can't register D-BUS handlers (fallback NPD). cannot continue.\n");
		return FALSE;
		
	}
	
	
	i = dbus_bus_request_name (wbmd_dbus_connection, WBMD_DBUS_BUSNAME,
			       0, &dbus_error);
		
	wbmd_syslog_debug_debug(WBMD_DBUS,"dbus_bus_request_name:%d",i);
	
	if (dbus_error_is_set (&dbus_error)) {
		wbmd_syslog_debug_debug(WBMD_DBUS,"dbus_bus_request_name(): %s",
			    dbus_error.message);
		return FALSE;
	}

	dbus_connection_add_filter (wbmd_dbus_connection, wbmd_dbus_filter_function, NULL, NULL);

	dbus_bus_add_match (wbmd_dbus_connection,
			    "type='signal'"
					    ",interface='"DBUS_INTERFACE_DBUS"'"
					    ",sender='"DBUS_SERVICE_DBUS"'"
					    ",member='NameOwnerChanged'",
			    NULL);
	
	return TRUE;
  
}

int wbmd_dbus_init(void)
{	
	int i = 0;
	DBusError dbus_error;
	dbus_threads_init_default();
	
	DBusObjectPathVTable	wbmd_vtable = {NULL, &wbmd_dbus_message_handler, NULL, NULL, NULL, NULL};	

	dbus_connection_set_change_sigpipe (TRUE);

	dbus_error_init (&dbus_error);
	wbmd_dbus_connection = dbus_bus_get_private (DBUS_BUS_SYSTEM, &dbus_error);

	if (wbmd_dbus_connection == NULL) {
		wbmd_syslog_err("dbus_bus_get(): %s\n", dbus_error.message);
		return FALSE;
	}

	if (!dbus_connection_register_fallback (wbmd_dbus_connection, WBMD_DBUS_OBJPATH, &wbmd_vtable, NULL)) {
		wbmd_syslog_err("can't register D-BUS handlers (fallback NPD). cannot continue.\n");
		return FALSE;
		
	}
	
	i = dbus_bus_request_name (wbmd_dbus_connection, WBMD_DBUS_BUSNAME,
			       0, &dbus_error);

	if (dbus_error_is_set (&dbus_error)) {
		wbmd_syslog_debug_debug(WBMD_DBUS,"dbus_bus_request_name(): %s",
			    dbus_error.message);
		return FALSE;
	}

	dbus_connection_add_filter (wbmd_dbus_connection, wbmd_dbus_filter_function, NULL, NULL);

	dbus_bus_add_match (wbmd_dbus_connection,
			    "type='signal'"
					    ",interface='"DBUS_INTERFACE_DBUS"'"
					    ",sender='"DBUS_SERVICE_DBUS"'"
					    ",member='NameOwnerChanged'",
			    NULL);
	
//	printf("init finished\n");
	return TRUE;
  
}

void *WBMDDbus()
{
	wbmd_pid_write_v2("WBMDDbus");	
	if(wbmd_dbus_init()
	){
		while (dbus_connection_read_write_dispatch(wbmd_dbus_connection,500)) {

		}
	}
	wbmd_syslog_err("there is something wrong in dbus handler\n");	

	return 0;
}

