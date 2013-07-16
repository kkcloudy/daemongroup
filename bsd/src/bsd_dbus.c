#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
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
#include <time.h>
#include "bsd/bsdpub.h"
#include "dbus/bsd/BsdDbusDef.h"
#include "bsd.h"
#include "bsd_dbus.h"
#include "bsd_thread.h"
#include "bsd_log.h"
#include "bsd_tipc.h"
#include "bsd_tcp.h"


static DBusConnection * bsd_dbus_connection = NULL;
static DBusConnection * bsd_dbus_connection2 = NULL;

extern unsigned short g_unEventId;

char g_rePrintMd5[BSD_PATH_LEN];


/** 
  * @brief  close socket
  * @param  conn   
  * @param  msg   
  * @param  user_data   
  * @return  
  * @author  zhangshu
  * @date  2012/06/07
  */
DBusMessage * bsd_close_tcp_socket(DBusConnection *conn, DBusMessage *msg, void *user_data){

    DBusMessage* reply;
    DBusMessageIter  iter;
    DBusError err;
    int iReturnValue = BSD_DBUS_SUCCESS;
    int iCurrentSocket = 0;
    dbus_error_init(&err);
    if (!(dbus_message_get_args ( msg, &err,
                             DBUS_TYPE_UINT32,&iCurrentSocket,
                             DBUS_TYPE_INVALID))) {
        bsd_syslog_err("copy files between boards:Unable to get input args ");
        if (dbus_error_is_set(&err)) {
            bsd_syslog_err("copy files failed %s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }
    
    /* close socket with socket id */
    iReturnValue = bsdCloseTcpSocket(iCurrentSocket);
   
    bsd_syslog_debug_debug(BSD_DEFAULT, "ret = %d\n", iReturnValue);
    
    reply = dbus_message_new_method_return(msg);
    if(NULL == reply) {
        bsd_syslog_err("vrrp set hansi profile dbus reply null!\n");
        return reply;
    }
    dbus_message_iter_init_append (reply, &iter);
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&iReturnValue);
    
    return reply;
    
}



/** 
  * @brief  check destination file path and free memery and return check result
  * @param  conn   
  * @param  msg   
  * @param  user_data   
  * @return  
  * @author  zhangshu
  * @date  2012/06/04
  */
DBusMessage * bsd_check_destination_device(DBusConnection *conn, DBusMessage *msg, void *user_data){

    DBusMessage* reply;
    DBusMessageIter  iter;
    DBusError err;
    int iReturnValue = BSD_DBUS_SUCCESS;
    int iDesAddr = 0;
    int iCompressFlag = 0;
    int iOption = 0;
    int iCurrentSocket = 0;
    char *pSrcPath = NULL;
    char *pDesPath = NULL;
    unsigned short ucEventId = 0;
    dbus_error_init(&err);
    if (!(dbus_message_get_args ( msg, &err,
                             DBUS_TYPE_UINT32,&iDesAddr,
                             DBUS_TYPE_STRING,&pSrcPath,
                             DBUS_TYPE_STRING,&pDesPath,
                             DBUS_TYPE_UINT32,&iCompressFlag,
                             DBUS_TYPE_UINT32,&iOption,
                             DBUS_TYPE_INVALID))) {
        bsd_syslog_err("copy files between boards:Unable to get input args ");
        if (dbus_error_is_set(&err)) {
            bsd_syslog_err("copy files failed %s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }
    
    if(g_unEventId == MAX_EVENT_ID) 
            g_unEventId = 0;
    g_unEventId++;
    ucEventId = g_unEventId;
    
    /* this is a remote file path check, iDesAddr is ip address */
    iReturnValue = bsdTcpCheckDestination(iDesAddr, pSrcPath, pDesPath, &iCurrentSocket);
    /* continue to send message */
    if(iReturnValue == BSD_SUCCESS) 
        iReturnValue = bsdTcpCopyFile(iCurrentSocket, pSrcPath, pDesPath, iCompressFlag, iOption);
    
    bsd_syslog_debug_debug(BSD_DEFAULT, "ret = %d, socket = %d\n", iReturnValue, iCurrentSocket);
    
    reply = dbus_message_new_method_return(msg);
    if(NULL == reply) {
        bsd_syslog_err("vrrp set hansi profile dbus reply null!\n");
        if(iCurrentSocket > 0)
            close(iCurrentSocket);
        return reply;
    }
    dbus_message_iter_init_append (reply, &iter);
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&iReturnValue);
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&iCurrentSocket);
    
    return reply;
}



/** 
  * @brief  check destination file path and free memery and return check result
  * @param  conn   
  * @param  msg   
  * @param  user_data   
  * @return  
  * @author  zhangshu
  * @date  2012/06/07
  */
DBusMessage * bsd_check_destination_board(DBusConnection *conn, DBusMessage *msg, void *user_data){

    DBusMessage* reply;
    DBusMessageIter  iter;
    DBusError err;
    int iReturnValue = BSD_DBUS_SUCCESS;
    int iSlotId = 0;
    int iCompressFlag = 0;
    int iOption = 0;
    char *pSrcPath = NULL;
    char *pDesPath = NULL;
    unsigned short ucEventId = 0;
    dbus_error_init(&err);
    if (!(dbus_message_get_args ( msg, &err,
                             DBUS_TYPE_UINT32,&iSlotId,
                             DBUS_TYPE_STRING,&pSrcPath,
                             DBUS_TYPE_STRING,&pDesPath,
                             DBUS_TYPE_UINT32,&iCompressFlag,   //book add, 2011-08-16
                             DBUS_TYPE_UINT32,&iOption,  //book add, 2011-10-26
                             DBUS_TYPE_INVALID))) {
        bsd_syslog_err("copy files between boards:Unable to get input args ");
        if (dbus_error_is_set(&err)) {
            bsd_syslog_err("copy files failed %s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }
    
    if(g_unEventId == MAX_EVENT_ID) 
            g_unEventId = 0;
    g_unEventId++;
    ucEventId = g_unEventId;

    if((iSlotId < 1) || (iSlotId >= MAX_SLOT_NUM)) {
        iReturnValue = BSD_DBUS_SLOT_ID_NOT_EXIST;
    }
    /* this is a local file path check, iDesAddr is slotId */
    else if((iReturnValue = BSDMemeryCheck(iSlotId, pSrcPath, pDesPath, ucEventId)) == 0) {
        iReturnValue = BSDDesPathCheck(iSlotId, pDesPath, ucEventId);
    }
    /* continue to send message */
    if(iReturnValue == BSD_SUCCESS) {
        BSD_BOARD[iSlotId]->state = BSD_FILE_UNKNOWN;
        iReturnValue = BSDCopyFilesToBoards(iSlotId, pSrcPath, pDesPath, iOption, iCompressFlag);
    }
    bsd_syslog_debug_debug(BSD_DEFAULT, "ret = %d\n", iReturnValue);
    
    reply = dbus_message_new_method_return(msg);
    if(NULL == reply) {
        bsd_syslog_err("vrrp set hansi profile dbus reply null!\n");
        return reply;
    }
    dbus_message_iter_init_append (reply, &iter);
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&iReturnValue);
    
    return reply;
}



/*****************************************************
** DISCRIPTION:
**          copy file of the specific path from slotA to slotB
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
** CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-08-11
*****************************************************/
DBusMessage * bsd_copy_files_between_boards_v2(DBusConnection *conn, DBusMessage *msg, void *user_data){

    DBusMessage* reply;
    DBusMessageIter  iter;
    DBusError err;
    int ret = BSD_DBUS_SUCCESS;
    unsigned int slotid = 0;
    int tar_switch = 0;
    char *src_path = NULL;
    char *des_path = NULL;
    int op = 0;
    dbus_error_init(&err);
    if (!(dbus_message_get_args ( msg, &err,
                             DBUS_TYPE_UINT32,&slotid,
                             DBUS_TYPE_STRING,&src_path,
                             DBUS_TYPE_STRING,&des_path,
                             DBUS_TYPE_UINT32,&tar_switch,   //book add, 2011-08-16
                             DBUS_TYPE_UINT32,&op,  //book add, 2011-10-26
                             DBUS_TYPE_INVALID))) {
        bsd_syslog_err("copy files between boards:Unable to get input args ");
        if (dbus_error_is_set(&err)) {
            bsd_syslog_err("copy files failed %s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }

    if((slotid < 1) || (slotid >= MAX_SLOT_NUM)) {
        ret = BSD_DBUS_SLOT_ID_NOT_EXIST;
    }
    else {
        BSD_BOARD[slotid]->state = BSD_FILE_UNKNOWN;
        ret = BSDCopyFilesToBoards(slotid, src_path, des_path, op, tar_switch);
        if((ret != 0) && (BSD_BOARD[slotid]->state != BSD_FILE_UNKNOWN)) {
            BSD_BOARD[slotid]->state = BSD_FILE_UNKNOWN;
        }
    }
    
    reply = dbus_message_new_method_return(msg);
    
    if(NULL == reply) {
        bsd_syslog_err("vrrp set hansi profile dbus reply null!\n");
        return reply;
    }
    
    dbus_message_iter_init_append (reply, &iter);
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
    
    return reply;
    
}



/*****************************************************
** DISCRIPTION:
**          copy file of the specific path from slotA to slotB
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
** CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-08-11
*****************************************************/
DBusMessage * bsd_copy_files_between_boards(DBusConnection *conn, DBusMessage *msg, void *user_data){

    DBusMessage* reply;
    DBusMessageIter  iter;
    DBusError err;
    int ret = BSD_DBUS_SUCCESS;
    unsigned int slotid = 0;
    int tar_switch = 0;
    char *src_path = NULL;
    char *des_path = NULL;
    int op = 0;
    DIR *source = NULL;
    int curr_event_id = 0;
    char *tmp_md5 = NULL;
    int len = 0;
    dbus_error_init(&err);
    if (!(dbus_message_get_args ( msg, &err,
                             DBUS_TYPE_UINT32,&slotid,
                             DBUS_TYPE_STRING,&src_path,
                             DBUS_TYPE_STRING,&des_path,
                             DBUS_TYPE_UINT32,&tar_switch,   //book add, 2011-08-16
                             DBUS_TYPE_UINT32,&op,  //book add, 2011-10-26
                             DBUS_TYPE_INVALID))) {
        bsd_syslog_err("copy files between boards:Unable to get input args ");
        if (dbus_error_is_set(&err)) {
            bsd_syslog_err("copy files failed %s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }

    if((ret != 0) && (BSD_BOARD[slotid]->state != BSD_FILE_UNKNOWN)) {
        BSD_BOARD[slotid]->state = BSD_FILE_UNKNOWN;
    }
    else {
        source = opendir(src_path);
        if(source == NULL) {    // copy single file
            if(op == BSD_TYPE_NORMAL)
                op = BSD_TYPE_SINGLE;
            
            if(g_unEventId == MAX_EVENT_ID) 
                g_unEventId = 0;
            g_unEventId++;
            curr_event_id = g_unEventId;
            bsd_syslog_debug_debug(BSD_DEFAULT, "111 op = %d, slotid = %d, src_path = %s, des_path = %s\n",op,slotid,src_path,des_path);
            ret = BSDHandleFileOp(slotid, src_path, des_path, op, curr_event_id);
            tarFlag = 0;
            
        } else {// copy folder
            bsd_syslog_debug_debug(BSD_DEFAULT, "copy other files\n");
            tarFlag = tar_switch;
            ret = BSDCopyFile(slotid, src_path, des_path, op);
            tarFlag = 0;
            closedir(source);
        }
        if((ret != 0) && (BSD_BOARD[slotid]->state != BSD_FILE_UNKNOWN)) {
            BSD_BOARD[slotid]->state = BSD_FILE_UNKNOWN;
        }
    }
    
    /* add for AXSSZFI-1563 */
    len = strnlen(g_rePrintMd5, BSD_PATH_LEN);
    //bsd_syslog_debug_debug(BSD_DEFAULT, "g_rePrintMd5 len = %d", len);
    if(len > 0) {
        tmp_md5 = (char*)malloc(len+1);
        memset(tmp_md5, 0, len+1);
        memcpy(tmp_md5, g_rePrintMd5, len);
		memset(g_rePrintMd5, 0, BSD_PATH_LEN);
    }
    else {
        tmp_md5 = (char*)malloc(5);
        memset(tmp_md5, 0, 5);
        strcpy(tmp_md5, "none");
    }
    
    bsd_syslog_debug_debug(BSD_DEFAULT, "ret = %d\n", ret);
    reply = dbus_message_new_method_return(msg);
    
    if(NULL == reply) {
        bsd_syslog_err("vrrp set hansi profile dbus reply null!\n");
        return reply;
    }
    
    dbus_message_iter_init_append (reply, &iter);
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_STRING,&tmp_md5);
    
    if(tmp_md5 != NULL) {
        free(tmp_md5);
        tmp_md5 = NULL;
    }
    return reply;
    
}


/** 
  * @brief  copy local files to another device
  * @param  conn   
  * @param  msg   
  * @param  user_data   
  * @return  
  * @author  zhangshu
  * @date  2012/05/28
  */
DBusMessage * bsd_copy_files_between_devices(DBusConnection *conn, DBusMessage *msg, void *user_data){

    DBusMessage* reply;
    DBusMessageIter  iter;
    DBusError err;
    int iReturnValue = BSD_DBUS_SUCCESS;
    int iTarFlag = 0;
    char *pSrcPath = NULL;
    char *pDesPath = NULL;
    int iOperation = 0;
    int iSocketId = 0;
    dbus_error_init(&err);
    if (!(dbus_message_get_args ( msg, &err,
                             DBUS_TYPE_UINT32,&iSocketId,
                             DBUS_TYPE_STRING,&pSrcPath,
                             DBUS_TYPE_STRING,&pDesPath,
                             DBUS_TYPE_UINT32,&iTarFlag,   //book add, 2011-08-16
                             DBUS_TYPE_UINT32,&iOperation,   //book add, 2011-10-26
                             DBUS_TYPE_INVALID))) {
        bsd_syslog_err("copy files between boards:Unable to get input args ");
        if (dbus_error_is_set(&err)) {
            bsd_syslog_err("copy files failed %s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }
    
    system("sudo mount /blk");
    iReturnValue = bsdTcpCopyFile(iSocketId, pSrcPath, pDesPath, iTarFlag, iOperation);
    system("sudo umount /blk");
    reply = dbus_message_new_method_return(msg);
    
    if(NULL == reply) {
        bsd_syslog_err("vrrp set hansi profile dbus reply null!\n");
        return reply;
    }
    dbus_message_iter_init_append (reply, &iter);
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&iReturnValue);
    
    return reply;
    
}




/*****************************************************
** DISCRIPTION:
**          synchronize files to other boards
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
** CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-08-18
*****************************************************/
DBusMessage * bsd_synchronize_files(DBusConnection *conn, DBusMessage *msg, void *user_data){

    DBusMessage* reply;
    DBusMessageIter  iter;
    DBusError err;
    int ret = BSD_DBUS_SUCCESS;
    int syn_type = 0;
    dbus_error_init(&err);
    if (!(dbus_message_get_args ( msg, &err,
                             DBUS_TYPE_UINT32,&syn_type,
                             DBUS_TYPE_INVALID))) 
    {
        bsd_syslog_err("copy files between boards:Unable to get input args ");
        if (dbus_error_is_set(&err)) {
            bsd_syslog_err("copy files failed %s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }

    
    system("sudo mount /blk");
    //printf("syn_type = %d\n",syn_type);
    ret = BSDSynchronizeFiles(syn_type);
    //printf("@@@@@@@@@  time = %ld @@@@@@@@@\n",finish - start);
    system("sudo umount /blk");
    reply = dbus_message_new_method_return(msg);
    if(NULL == reply){      
        bsd_syslog_err("vrrp set hansi profile dbus reply null!\n");
        return reply;
    }
    
    dbus_message_iter_init_append (reply, &iter);
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
    
    return reply;
    
}


/*****************************************************
** DISCRIPTION:
**          synchronize wtp version or certification
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
** CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-09-01
*****************************************************/
DBusMessage * bsd_synchronize_files_v2(DBusConnection *conn, DBusMessage *msg, void *user_data){

    DBusMessage* reply;
    DBusMessageIter  iter;
    DBusError err;
    int ret = BSD_DBUS_SUCCESS;
    char *src_path = NULL;
    char *des_path = NULL;
	int op = BSD_TYPE_NORMAL;
    dbus_error_init(&err);
    if (!(dbus_message_get_args ( msg, &err,
                             DBUS_TYPE_STRING,&src_path,
                             DBUS_TYPE_STRING,&des_path,
                             DBUS_TYPE_UINT32,&op,
                             DBUS_TYPE_INVALID))) 
    {
        bsd_syslog_err("copy files between boards:Unable to get input args ");
        if (dbus_error_is_set(&err)) {
            bsd_syslog_err("copy files failed %s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }

    
    system("sudo mount /blk");
    ret = BSDSynchronizeFilesV2(src_path, des_path, op);
    system("sudo umount /blk");
    reply = dbus_message_new_method_return(msg);
    if(NULL == reply){      
        bsd_syslog_err("vrrp set hansi profile dbus reply null!\n");
        return reply;
    }
    
    dbus_message_iter_init_append (reply, &iter);
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
    
    return reply;
    
}



/*****************************************************
** DISCRIPTION:
**          bsd get all alive slot ids
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
** CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-10-24
*****************************************************/
DBusMessage * bsd_get_alive_slot_ids(DBusConnection *conn, DBusMessage *msg, void *user_data){

    DBusMessage* reply;
    DBusMessageIter  iter;
    DBusError err;
    int ret = 0;
	int ID[MAX_SLOT_NUM] = {0};
	int i = 0;
	int op = 0;
	
    dbus_error_init(&err);
    if (!(dbus_message_get_args ( msg, &err,
                             DBUS_TYPE_UINT32,&op,
                             DBUS_TYPE_INVALID))) 
    {
        bsd_syslog_err("copy files between boards:Unable to get input args ");
        if (dbus_error_is_set(&err)) {
            bsd_syslog_err("copy files failed %s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }
    
    ret = BSDGetAliveSlotIDs(ID, op);
    //printf("ret = %d\n",ret);
    reply = dbus_message_new_method_return(msg);
    if(NULL == reply){
        bsd_syslog_err("vrrp set hansi profile dbus reply null!\n");
        return reply;
    }
    
    dbus_message_iter_init_append (reply, &iter);
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
    if(ret != 0)
    {
        for(i = 0; i < ret; i++)
        {
            //printf("slotid = %d\n",ID[i]);
            dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&(ID[i]));
        }
    }
    
    return reply;
    
}



/*****************************************************
** DISCRIPTION:
**          bsd set daemonlog level
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
DBusMessage * bsd_set_daemonlog_level(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusError err;
	dbus_error_init(&err);
	char loglevel[20];	
	int ret = BSD_DBUS_SUCCESS;
	unsigned int daemonlogtype = 0;
	unsigned int daemonloglevel = 0;

	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&daemonlogtype,
								DBUS_TYPE_UINT32,&daemonloglevel,
								DBUS_TYPE_INVALID)))
    {
		bsd_syslog_err("Unable to get input args\n");
		if (dbus_error_is_set(&err)) 
		{
			bsd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);

	if(daemonloglevel == 1)	
	{
		BsdLOGLEVEL |= daemonlogtype;
		strcpy(loglevel, "open");
	}	
	else if(daemonloglevel == 0)	
	{
		BsdLOGLEVEL &= ~daemonlogtype;
		strcpy(loglevel, "close");
	}
	bsd_syslog_debug_debug(BSD_DEFAULT,"set BSD daemonlog debug %d\n",BsdLOGLEVEL);
	return reply;	
    
}



static DBusHandlerResult bsd_dbus_message_handler (DBusConnection *connection, DBusMessage *message, void *user_data){

	DBusMessage		*reply = NULL;

	if(message == NULL)
		return DBUS_HANDLER_RESULT_HANDLED;

	if	(strcmp(dbus_message_get_path(message),BSD_DBUS_OBJPATH) == 0)	{		
		if (dbus_message_is_method_call(message,BSD_DBUS_INTERFACE,BSD_COPY_FILES_BETEWEEN_BORADS)) {
			reply = bsd_copy_files_between_boards(connection,message,user_data);
		} 
		else if (dbus_message_is_method_call(message,BSD_DBUS_INTERFACE,BSD_SYNCHRONIZE_FILES_TO_OTHER_BOARDS)) {
			reply = bsd_synchronize_files(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,BSD_DBUS_INTERFACE,BSD_SYNCHRONIZE_FILES_TO_OTHER_BOARDS_V2)) {
			reply = bsd_synchronize_files_v2(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,BSD_DBUS_INTERFACE,BSD_GET_ALIVE_SLOT_IDS)) {
			reply = bsd_get_alive_slot_ids(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,BSD_DBUS_INTERFACE,BSD_SET_BSD_DAEMONLOG_LEVEL)) {
			reply = bsd_set_daemonlog_level(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,BSD_DBUS_INTERFACE,BSD_COPY_FILES_BETEWEEN_DEVICES)) {
			reply = bsd_copy_files_between_devices(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,BSD_DBUS_INTERFACE,BSD_CHECK_DETINATION_BOARD_INFORMATION)) {
			reply = bsd_check_destination_board(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,BSD_DBUS_INTERFACE,BSD_CHECK_DETINATION_DEVICE_INFORMATION)) {
			reply = bsd_check_destination_device(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,BSD_DBUS_INTERFACE,BSD_CLOSE_TCP_SOCKET)) {
			reply = bsd_close_tcp_socket(connection,message,user_data);
		}
		if (dbus_message_is_method_call(message,BSD_DBUS_INTERFACE,BSD_COPY_FILES_BETEWEEN_BORADS_V2)) {
			reply = bsd_copy_files_between_boards_v2(connection,message,user_data);
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
bsd_dbus_filter_function (DBusConnection * connection,
					   DBusMessage * message, void *user_data)
{
	if (dbus_message_is_signal (message, DBUS_INTERFACE_LOCAL, "Disconnected") &&
		   strcmp (dbus_message_get_path (message), DBUS_PATH_LOCAL) == 0) {

		bsd_syslog_err("Got disconnected from the system message bus; "
				"retrying to reconnect every 3000 ms\n");
		dbus_connection_close(bsd_dbus_connection);
	    bsd_dbus_connection = NULL;
		BsdThread thread_dbus; 
		if(!(BsdCreateThread(&thread_dbus, bsd_dbus_thread_restart, NULL,0))) {
			bsd_syslog_crit("Error starting Dbus Thread");
			exit(1);
		}

	} else if (dbus_message_is_signal (message,
			      DBUS_INTERFACE_DBUS,
			      "NameOwnerChanged")) {

	} else
		return TRUE;

	return DBUS_HANDLER_RESULT_HANDLED;
}

int bsd_dbus_reinit(void)
{	
	int i = 0;
	DBusError dbus_error;
	dbus_threads_init_default();
	
	DBusObjectPathVTable	bsd_vtable = {NULL, &bsd_dbus_message_handler, NULL, NULL, NULL, NULL};	

//	printf("npd dbus init\n");
	dbus_connection_set_change_sigpipe (TRUE);

	dbus_error_init (&dbus_error);
	bsd_dbus_connection = dbus_bus_get_private (DBUS_BUS_SYSTEM, &dbus_error);
	if (bsd_dbus_connection == NULL) {
		bsd_syslog_err("dbus_bus_get(): %s\n", dbus_error.message);
		return FALSE;
	}

	// Use npd to handle subsection of NPD_DBUS_OBJPATH including slots
	if (!dbus_connection_register_fallback (bsd_dbus_connection, BSD_DBUS_OBJPATH, &bsd_vtable, NULL)) {
		bsd_syslog_err("can't register D-BUS handlers (fallback NPD). cannot continue.\n");
		return FALSE;
	}
	
	i = dbus_bus_request_name (bsd_dbus_connection, BSD_DBUS_BUSNAME,
			       0, &dbus_error);
		
	bsd_syslog_debug_debug(BSD_DBUS,"dbus_bus_request_name:%d",i);
	
	if (dbus_error_is_set (&dbus_error)) {
		bsd_syslog_debug_debug(BSD_DBUS,"dbus_bus_request_name(): %s",
			    dbus_error.message);
		return FALSE;
	}

	dbus_connection_add_filter (bsd_dbus_connection, bsd_dbus_filter_function, NULL, NULL);

	dbus_bus_add_match (bsd_dbus_connection,
			    "type='signal'"
					    ",interface='"DBUS_INTERFACE_DBUS"'"
					    ",sender='"DBUS_SERVICE_DBUS"'"
					    ",member='NameOwnerChanged'",
			    NULL);
	
	return TRUE;
  
}

int bsd_dbus_init(void)
{	
	int i = 0;
	DBusError dbus_error;
	dbus_threads_init_default();
	
	DBusObjectPathVTable	bsd_vtable = {NULL, &bsd_dbus_message_handler, NULL, NULL, NULL, NULL};	

	dbus_connection_set_change_sigpipe (TRUE);

	dbus_error_init (&dbus_error);
	bsd_dbus_connection = dbus_bus_get_private (DBUS_BUS_SYSTEM, &dbus_error);
	bsd_dbus_connection2 = dbus_bus_get_private (DBUS_BUS_SYSTEM, &dbus_error);

	if (bsd_dbus_connection == NULL) {
		bsd_syslog_err("dbus_bus_get(): %s\n", dbus_error.message);
		return FALSE;
	}
    
	if (!dbus_connection_register_fallback (bsd_dbus_connection, BSD_DBUS_OBJPATH, &bsd_vtable, NULL)) {
		bsd_syslog_err("can't register D-BUS handlers (fallback NPD). cannot continue.\n");
		return FALSE;
		
	}
	
	i = dbus_bus_request_name (bsd_dbus_connection, BSD_DBUS_BUSNAME,
			       0, &dbus_error);
	dbus_bus_request_name (bsd_dbus_connection2, "aw.bsd2",
			       0, &dbus_error);

	if (dbus_error_is_set (&dbus_error)) {
		bsd_syslog_debug_debug(BSD_DBUS,"dbus_bus_request_name(): %s",
			    dbus_error.message);
		return FALSE;
	}

	dbus_connection_add_filter (bsd_dbus_connection, bsd_dbus_filter_function, NULL, NULL);

	dbus_bus_add_match (bsd_dbus_connection,
			    "type='signal'"
					    ",interface='"DBUS_INTERFACE_DBUS"'"
					    ",sender='"DBUS_SERVICE_DBUS"'"
					    ",member='NameOwnerChanged'",
			    NULL);
	
//	printf("init finished\n");
	return TRUE;
  
}

void *bsdDbus()
{
	bsd_pid_write_v2("BSDDbus",HOST_SLOT_NO);	
	if(bsd_dbus_init()) {
		while (dbus_connection_read_write_dispatch(bsd_dbus_connection,500)) {

		}
	}
	bsd_syslog_err("there is something wrong in dbus handler\n");	

	return 0;
}

void *bsd_dbus_thread_restart()
{
	bsd_pid_write_v2("BSDDbus Reinit",HOST_SLOT_NO);	
	if(bsd_dbus_reinit()) {
		while (dbus_connection_read_write_dispatch(bsd_dbus_connection,500)) {

		}
	}
	bsd_syslog_err("there is something wrong in dbus handler\n");	
	return 0;
}

#undef _GNU_SOURCE

