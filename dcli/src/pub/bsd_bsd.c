#ifdef _D_WCPSS_

#include <dbus/dbus.h>
#include "bsd/bsdpub.h"
#include "dbus/bsd/BsdDbusPath.h"
#include "dbus/bsd/BsdDbusDef.h"
//#include "../lib/dcli_main.h"
#include "bsd_bsd.h"

/*****************************************************
** DISCRIPTION:
**          copy specific file to other boards
** INPUT:
**          slotA_path
**          slotB_path
** OUTPUT:
**          
** RETURN:
**          
** CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-09-01
*****************************************************/
int dcli_bsd_copy_files_to_boards(DBusConnection *connection,const char *src_path, const char *des_path, const int op)
{
    int ret = 0;
    DBusMessage *query = NULL;
    DBusMessage *reply = NULL;
    DBusError err = {0};
	int retu = 0;
    
    char *tmp_src_path = src_path;
    char *tmp_des_path = des_path;

    query = dbus_message_new_method_call(BSD_DBUS_BUSNAME, BSD_DBUS_OBJPATH, \
        BSD_DBUS_INTERFACE, BSD_SYNCHRONIZE_FILES_TO_OTHER_BOARDS_V2);

    dbus_message_append_args(query,
    					DBUS_TYPE_STRING,&tmp_src_path,
    					DBUS_TYPE_STRING,&tmp_des_path,
    					DBUS_TYPE_UINT32,&op,
    					DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block (connection,query,60000000, &err);

    dbus_message_unref(query);

    if (NULL == reply) {
    	//printf("<error> failed get reply.\n");
    	if (dbus_error_is_set(&err)) {
    		//printf("%s raised: %s",err.name,err.message);
    		dbus_error_free(&err);
    	}
    	return CMD_SUCCESS;
    }

    if (dbus_message_get_args ( reply, &err,
    				DBUS_TYPE_UINT32,&ret,
    				DBUS_TYPE_INVALID)) {
    	if(ret == 0){
    		//printf("Synchronize successfully\n", src_path);
    		retu = 1;
    	}
    	else
    		//printf("Synchronize file error.\n");
    		retu = 0;
    }

    return ret;
}


/*****************************************************
** DISCRIPTION:
**          copy files from master boards to another
** INPUT:
**          slotB_id
**          slotA_path
**          slotB_path
** OUTPUT:
**          
** RETURN:
**          
** CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-10-26
*****************************************************/
int dcli_bsd_copy_file_to_board(DBusConnection *connection, const int slot_id, const char *src_path, const char *des_path, const int flag, const int op)
{   
    int ret = 0;
    DBusMessage *query = NULL;
    DBusMessage *reply = NULL;
    DBusError err = {0};
	int retu = 0;
    
    char *tmp_src_path = src_path;
    char *tmp_des_path = des_path;
    
    //printf("dbus_name = %s\ndbus_objpath = %s\ndbus_interface = %s\n",BSD_DBUS_BUSNAME,BSD_DBUS_OBJPATH,BSD_DBUS_INTERFACE);
    query = dbus_message_new_method_call(BSD_DBUS_BUSNAME,BSD_DBUS_OBJPATH,\
						BSD_DBUS_INTERFACE,BSD_COPY_FILES_BETEWEEN_BORADS);
    
	dbus_error_init(&err);
    //printf("11111\n");
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&slot_id,
							 DBUS_TYPE_STRING,&tmp_src_path,
							 DBUS_TYPE_STRING,&tmp_des_path,
							 DBUS_TYPE_UINT32,&flag,
							 DBUS_TYPE_UINT32,&op,
							 DBUS_TYPE_INVALID);
    
    printf("Copying file, please wait ...\n");
    //printf("0000\n");
	reply = dbus_connection_send_with_reply_and_block (connection,query,120000, &err);
	//printf("0001\n");
	dbus_message_unref(query);
	//printf("2222\n");
	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	//printf("3333\n");
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&ret,
					DBUS_TYPE_INVALID)) {
		
	}
	//printf("4444\n");
	dbus_message_unref(reply);
    
    return ret;
}



/*****************************************************
** DISCRIPTION:
**          copy files from master boards to another
** INPUT:
**          slotB_id
**          slotA_path
**          slotB_path
** OUTPUT:
**          
** RETURN:
**          
** CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-10-26
*****************************************************/
int dcli_bsd_copy_file_to_board_v2(DBusConnection *connection, const int slot_id, const char *src_path, const char *des_path, const int flag, const int op)
{   
    int ret = 0;
    DBusMessage *query = NULL;
    DBusMessage *reply = NULL;
    DBusError err = {0};
	int retu = 0;
    
    char *tmp_src_path = src_path;
    char *tmp_des_path = des_path;
    
    //printf("dbus_name = %s\ndbus_objpath = %s\ndbus_interface = %s\n",BSD_DBUS_BUSNAME,BSD_DBUS_OBJPATH,BSD_DBUS_INTERFACE);
    query = dbus_message_new_method_call(BSD_DBUS_BUSNAME,BSD_DBUS_OBJPATH,\
						BSD_DBUS_INTERFACE,BSD_COPY_FILES_BETEWEEN_BORADS_V2);
    
	dbus_error_init(&err);
    //printf("11111\n");
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&slot_id,
							 DBUS_TYPE_STRING,&tmp_src_path,
							 DBUS_TYPE_STRING,&tmp_des_path,
							 DBUS_TYPE_UINT32,&flag,
							 DBUS_TYPE_UINT32,&op,
							 DBUS_TYPE_INVALID);
    
    printf("Copying file, please wait ...\n");
    //printf("0000\n");
	reply = dbus_connection_send_with_reply_and_block (connection,query,60000, &err);
	//printf("0001\n");
	dbus_message_unref(query);
	//printf("2222\n");
	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	//printf("3333\n");
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&ret,
					DBUS_TYPE_INVALID)) {
		
	}
	//printf("4444\n");
	dbus_message_unref(reply);
    
    return ret;
}




/*****************************************************
** DISCRIPTION:
**          get alive slot ids
** INPUT:
**          op
** OUTPUT:
**          slot ids
** RETURN:
**          ret
** CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-10-24
*****************************************************/
int dcli_bsd_get_slot_ids(DBusConnection *connection, int *ID, const int op)
{
    int ret = 0;
    DBusMessageIter  iter;
    DBusMessage *query = NULL;
    DBusMessage *reply = NULL;
    DBusError err = {0};
	int i = 0;
	int slot_id = 0;
   

    query = dbus_message_new_method_call(BSD_DBUS_BUSNAME, BSD_DBUS_OBJPATH, \
        BSD_DBUS_INTERFACE, BSD_GET_ALIVE_SLOT_IDS);

    dbus_message_append_args(query,
    					DBUS_TYPE_UINT32,&op,
    					DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);

    dbus_message_unref(query);

    if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return -1;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	//printf("ret = %d\n",ret);

	if(ret != 0){
		for(i = 0; i < ret; i++)
		{
		    dbus_message_iter_next(&iter);
	        dbus_message_iter_get_basic(&iter,&(ID[i]));
		}
	}

    return ret;
}


/*****************************************************
** DISCRIPTION:
**          set bsd daemon log level
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
** CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2012-2-8
*****************************************************/
int dcli_set_bsd_daemonlog(int index, unsigned int daemonlogtype, unsigned int daemonloglevel, DBusConnection *dbus_connection, char *DBUS_METHOD)
{
    DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	int ret = 0;	
	
	query = dbus_message_new_method_call(BSD_DBUS_BUSNAME,BSD_DBUS_OBJPATH,BSD_DBUS_INTERFACE,DBUS_METHOD);
	
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




/** 
  * @brief  
  * @param  connection   
  * @param  iSocketId   
  * @param  src_path   
  * @param  des_path   
  * @param  flag   
  * @param  op   
  * @return  
  * @author  zhangshu
  * @date  2012/06/07
  */
int dcli_bsd_copy_file_to_device(DBusConnection *connection, const int iSocketId, const char *src_path, const char *des_path, const int flag, const int op)
{   
    int ret = 0;
    DBusMessage *query = NULL;
    DBusMessage *reply = NULL;
    DBusError err = {0};
	int retu = 0;
    
    char *tmp_src_path = src_path;
    char *tmp_des_path = des_path;
    
    //printf("dbus_name = %s\ndbus_objpath = %s\ndbus_interface = %s\n",BSD_DBUS_BUSNAME,BSD_DBUS_OBJPATH,BSD_DBUS_INTERFACE);
    query = dbus_message_new_method_call(BSD_DBUS_BUSNAME,BSD_DBUS_OBJPATH,\
						BSD_DBUS_INTERFACE,BSD_COPY_FILES_BETEWEEN_DEVICES);
    
	dbus_error_init(&err);
    
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&iSocketId,
							 DBUS_TYPE_STRING,&tmp_src_path,
							 DBUS_TYPE_STRING,&tmp_des_path,
							 DBUS_TYPE_UINT32,&flag,
							 DBUS_TYPE_UINT32,&op,
							 DBUS_TYPE_INVALID);
    
	reply = dbus_connection_send_with_reply_and_block (connection,query,120000, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&ret,
					DBUS_TYPE_INVALID)) {
		
	}
	dbus_message_unref(reply);
    
    return ret;
}





/** 
  * @brief  
  * @param  connection   
  * @param  iDesAddr   
  * @param  src_path   
  * @param  des_path   
  * @param  flag   
  * @param  iOption   
  * @param  piSocketId   
  * @return  
  * @author  zhangshu
  * @date  2012/06/07
  */
int dcli_bsd_check_destination_device_information(DBusConnection *connection, const int iDesAddr, const char *src_path, const char *des_path, const int flag, const int iOption, int *piSocketId)
{   
    int ret = 0;
    DBusMessage *query = NULL;
    DBusMessage *reply = NULL;
    DBusError err = {0};
	int retu = 0;
    
    char *tmp_src_path = src_path;
    char *tmp_des_path = des_path;
    
    //printf("dbus_name = %s\ndbus_objpath = %s\ndbus_interface = %s\n",BSD_DBUS_BUSNAME,BSD_DBUS_OBJPATH,BSD_DBUS_INTERFACE);
    query = dbus_message_new_method_call(BSD_DBUS_BUSNAME,BSD_DBUS_OBJPATH,\
						BSD_DBUS_INTERFACE,BSD_CHECK_DETINATION_DEVICE_INFORMATION);
    
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&iDesAddr,
							 DBUS_TYPE_STRING,&tmp_src_path,
							 DBUS_TYPE_STRING,&tmp_des_path,
							 DBUS_TYPE_UINT32,&flag,
							 DBUS_TYPE_UINT32,&iOption,
							 DBUS_TYPE_INVALID);
    
    
	reply = dbus_connection_send_with_reply_and_block (connection,query,120000, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&ret,
					DBUS_TYPE_UINT32,piSocketId,
					DBUS_TYPE_INVALID)) {
		
	}
	//printf("socket id = %d\n", *piSocketId);
	dbus_message_unref(reply);
    
    return ret;
}



/** 
  * @brief  
  * @param  connection   
  * @param  iDesAddr   
  * @param  src_path   
  * @param  des_path   
  * @param  flag   
  * @param  iOption   
  * @return  
  * @author  zhangshu
  * @date  2012/06/07
  */
int dcli_bsd_check_destination_board_information(DBusConnection *connection, const int iSlotId, const char *src_path, const char *des_path, const int flag, const int iOption)
{   
    int ret = 0;
    DBusMessage *query = NULL;
    DBusMessage *reply = NULL;
    DBusError err = {0};
	int retu = 0;
    
    char *tmp_src_path = src_path;
    char *tmp_des_path = des_path;
    
    //printf("dbus_name = %s\ndbus_objpath = %s\ndbus_interface = %s\n",BSD_DBUS_BUSNAME,BSD_DBUS_OBJPATH,BSD_DBUS_INTERFACE);
    query = dbus_message_new_method_call(BSD_DBUS_BUSNAME,BSD_DBUS_OBJPATH,\
						BSD_DBUS_INTERFACE,BSD_CHECK_DETINATION_BOARD_INFORMATION);
    
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&iSlotId,
							 DBUS_TYPE_STRING,&tmp_src_path,
							 DBUS_TYPE_STRING,&tmp_des_path,
							 DBUS_TYPE_UINT32,&flag,
							 DBUS_TYPE_UINT32,&iOption,
							 DBUS_TYPE_INVALID);
    
	reply = dbus_connection_send_with_reply_and_block (connection,query,60000, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&ret,
					DBUS_TYPE_INVALID)) {
		
	}
	
	dbus_message_unref(reply);
    
    return ret;
}



/** 
  * @brief  
  * @param  connection   
  * @param  iSocketId   
  * @return  
  * @author  zhangshu
  * @date  2012/06/07
  */
int dcli_bsd_close_tcp_socket(DBusConnection *connection, const int iSocketId)
{
    DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	int ret = 0;	
	
	query = dbus_message_new_method_call(BSD_DBUS_BUSNAME,BSD_DBUS_OBJPATH,\
						BSD_DBUS_INTERFACE,BSD_CLOSE_TCP_SOCKET);
    
	dbus_error_init(&err);

    dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&iSocketId,
							 DBUS_TYPE_INVALID);
    
	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	
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



#endif