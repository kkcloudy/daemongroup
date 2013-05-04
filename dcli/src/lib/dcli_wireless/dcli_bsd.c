#ifdef _D_WCPSS_

#include <string.h>
#include <zebra.h>
#include "vtysh/vtysh.h"
#include <dbus/dbus.h>
#include "command.h"
#include <dbus/dbus.h>
#include "bsd/bsdpub.h"
#include "dbus/bsd/BsdDbusPath.h"
#include "dbus/bsd/BsdDbusDef.h"
#include "../dcli_main.h"
#include "dcli_bsd.h"
#include "bsd_bsd.h"


/** 
  * @brief  
  * @param  str   
  * @param  ID   
  * @return  
  * @author  zhangshu
  * @date  2012/05/25
  */
static int parse_int_ID(char* str,unsigned int* ID){
	char *endptr = NULL;
	char c;
	c = str[0];
	if (c>='0'&&c<='9'){
		*ID= strtoul(str,&endptr,10);
		if((c=='0')&&(str[1]!='\0')){
			 return -1;
		}
		else if((endptr[0] == '\0')||(endptr[0] == '\n')){
			return 0;
		}
		else{
			return -1;
		}
	}
	else{
		return -1;
	}
}



/** 
  * @brief  generate return string with return value
  * @param  iReturnValue   
  * @param  pReturnStr   
  * @return  
  * @author  zhangshu
  * @date  2012/06/04
  */
char * dcli_bsd_get_return_string(int iReturnValue, char pReturnStr[BSD_COMMAND_BUF_LEN])
{
    if(pReturnStr == NULL)
        return "Copy file error.\n";
    memset(pReturnStr, 0, BSD_COMMAND_BUF_LEN);
    switch(iReturnValue) {
        case BSD_SUCCESS:
            memcpy(pReturnStr, "Copy file successful.\n", strlen("Copy file successful.\n"));
            break;
        case BSD_GET_SLOT_INFORMATION_ERROR:
            memcpy(pReturnStr, "Get slot information error.\n", strlen("Get slot information error\n"));
            break;
        case BSD_INIT_SOCKET_ERROR:
        case BSD_ESTABLISH_CONNECTION_FAILED:
            memcpy(pReturnStr, "Failed to connect with target device.\n", strlen("Failed to connect with target device.\n"));
            break;
        case BSD_ILLEGAL_SOURCE_FILE_PATH:
            memcpy(pReturnStr, "Illegal source file path.\n", strlen("Illegal source file path.\n"));
            break;
        case BSD_ILLEGAL_DESTINATION_FILE_PATH:
            memcpy(pReturnStr, "Illegal target file path.\n", strlen("Illegal target file path.\n"));
            break;
        case BSD_GET_FILE_SIZE_ERROR:
            memcpy(pReturnStr, "Failed to get file size.\n", strlen("Failed to get file size.\n"));
            break;
        case BSD_NOT_ENOUGH_MEMERY:
            memcpy(pReturnStr, "Not enough free memery on target device.\n", strlen("Not enough free memery on target device.\n"));
            break;
        case BSD_SEND_MESSAGE_ERROR:
            memcpy(pReturnStr, "Failed to send message to target device.\n", strlen("Failed to send message to target device.\n"));
            break;
        case BSD_RECEIVE_MESSAGE_ERROR:
            memcpy(pReturnStr, "Failed to receive message from target device.\n", strlen("Failed to receive message from target device.\n"));
            break;
        case BSD_EVENTID_NOT_MATCH:
            memcpy(pReturnStr, "Sending event ID not match.\n", strlen("Sending event ID not match.\n"));
            break;
        case BSD_PEER_SAVE_FILE_ERROR:
            memcpy(pReturnStr, "Failed to save file on target device.\n", strlen("Failed to save file on target device.\n"));
            break;
        case BSD_SERVER_NOT_CATCH:
            memcpy(pReturnStr, "Can not find target device.\n", strlen("Can not find target device.\n"));
            break;
        case BSD_MALLOC_MEMERY_ERROR:
            memcpy(pReturnStr, "Malloc memery error.\n", strlen("Malloc memery error.\n"));
            break;
        case BSD_MD5_ERROR:
            memcpy(pReturnStr, "MD5 check error.\n", strlen("MD5 check error.\n"));
            break;
		case BSD_MD5_ERROR_THIRD:
            memcpy(pReturnStr, "MD5 check error third,you can try again.\n", strlen("MD5 check error third,you can try again.\n"));
            break;
        case BSD_WAIT_THREAD_CONDITION_TIMEOUT:
            memcpy(pReturnStr, "Wait for peer response over time.\n", strlen("Wait for peer response over time.\n"));
            break;
        case BSD_ADD_TO_MESSAGE_QUEUE_ERROR:
            memcpy(pReturnStr, "Failed to add message into msgq.\n", strlen("Failed to add message into msgq.\n"));
            break;
        case BSD_UNKNOWN_ERROR:
        default:
            memcpy(pReturnStr, "Copy file failed with unknow error.\n", strlen("Copy file failed with unknow error.\n"));
            break;
    }
    return (char*)pReturnStr;
}


/** 
  * @brief  seperate ip address & file path from full path
  * @param  p_desFullPath   
  * @param  p_nDesAddress   
  * @param  p_desFilePath   
  * @return  
  * @author  zhangshu
  * @date  2012/05/25
  */
static int dcliParseDesPath(const char * p_desFullPath, char *p_nDesAddress, char * p_desFilePath)
{
    /** 
        * p_desFullPath is like "10.0.1.100:/mnt/wtp/wtpconfig.xml"
        * p_nDesAddress is like "10.0.1.100"
        * p_desFilePath is like "/mnt/wtp"
        */
    int iReturnValue = CMD_SUCCESS;
    char * p_colonPos = NULL;
    
    p_colonPos = strchr(p_desFullPath, ':');
    if(p_colonPos == NULL)
        return -1;
    
    //printf("colon position is %d\n", p_colonPos-p_desFullPath);
    strncpy(p_nDesAddress, p_desFullPath, p_colonPos-p_desFullPath);
    //printf("11\n");
    strncpy(p_desFilePath, p_colonPos+1, (strlen(p_desFullPath)-(p_colonPos-p_desFullPath)));
    //printf("ip address is %s\n", p_nDesAddress);
    //printf("file path is %s\n", p_desFilePath);
    
    return iReturnValue;
}



/*****************************************************
** DISCRIPTION:
**          copy file of the specific path from slotA to slotB
** INPUT:
**          slotA_ID
**          slotA_path
**          slotB_ID
**          slotB_path
** OUTPUT:
**          
** RETURN:
**          
** CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-08-11
*****************************************************/
DEFUN(copy_files_between_boards_func,
	  copy_files_between_boards_cmd,
	  "copy SSLOT SPATH to DSLOT DPATH",
	  "copy files from slot1 path1 to slot2 path2\n"
	  "copy 5 /wtp/mnt/wtp/wtpcompatible.xml to 2 /wtp/mnt/wtp/wtpcompatible.xml\n"
	  "copy 5 /wtp/mnt/wtp/wtpcompatible.xml to 2 /wtp/mnt/wtp/wtpcompatible.xml\n"
	  "copy 5 /wtp/mnt/wtp/wtpcompatible.xml to 2 /wtp/mnt/wtp/wtpcompatible.xml\n"
	  "copy 5 /wtp/mnt/wtp/wtpcompatible.xml to 2 /wtp/mnt/wtp/wtpcompatible.xml\n"
	 )
{
	int ret = 0;
	int src_len = 0;
	int des_len = 0;
	unsigned int src_slotid = 0;
	unsigned int des_slotid = 0;
	char src_path[PATH_LEN] = {0};
	char des_path[PATH_LEN] = {0};
	char sel[BSD_COMMAND_BUF_LEN] = {0};
	char a_returnString[BSD_COMMAND_BUF_LEN] = {0};
    
    ret = parse_int_ID((char *)argv[0], &src_slotid);
    
    if(ret != 0){
        vty_out(vty,"<error> unknown slotid format\n");
        return CMD_SUCCESS;
    }
    ret = parse_int_ID((char *)argv[2], &des_slotid);
    
    if(ret != 0){
        vty_out(vty,"<error> unknown slotid format\n");
        return CMD_SUCCESS;
    }
    
	src_len = strlen(argv[1]);
	des_len = strlen(argv[3]);
	
	if((src_len >= 64) || (des_len >= 64)){		
		vty_out(vty,"<error> file path name is too long,should be less than 64\n");
		return CMD_SUCCESS;
	}
	memcpy(src_path, argv[1], src_len);
	memcpy(des_path, argv[3], des_len);
    
	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,src_slotid,distributFag);

	if(dcli_dbus_connection == NULL)
	    return CMD_SUCCESS;
	
	ret = dcli_bsd_check_destination_board_information(dcli_dbus_connection,des_slotid,src_path, des_path, 1, BSD_TYPE_NORMAL);
	if(ret != BSD_SUCCESS) {
	    if(ret == BSD_ILLEGAL_DESTINATION_FILE_PATH) {
	        /* force to create destination path? */
			vty_out(vty,"Destination path is not exist, forced to create? [yes/no]:\n");
			fscanf(stdin, "%s", sel);
	        while(1) {
				if(!strncasecmp("yes", sel, strlen(sel))) {
				    ret = dcli_bsd_copy_file_to_board_v2(dcli_dbus_connection,des_slotid,src_path,des_path,1,BSD_TYPE_NORMAL);
					break;
				} else if (!strncasecmp("no", sel, strlen(sel))) {
				    vty_out(vty, "Operation has been canceled.\n");
					return CMD_SUCCESS;
				} else {
					vty_out(vty,"% Please input 'yes' or 'no'.\n");
					vty_out(vty,"Destination path is not exist, forced to create? [yes/no]:\n");
					memset(sel, 0, PATH_LEN);
					fscanf(stdin, "%s", sel);
				}
			}
	    } else {
	        vty_out(vty,"%s", dcli_bsd_get_return_string(ret, a_returnString));
	        return CMD_SUCCESS;
	    }
	}
    
    vty_out(vty,"%s", dcli_bsd_get_return_string(ret, a_returnString));
	//printf("5555\n");
	return CMD_SUCCESS;			
}


/*****************************************************
** DISCRIPTION:
**          synchronization /blk files from master board to others
** INPUT:
**          
** OUTPUT:
**          result
** RETURN:
**          
** CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-10-26
*****************************************************/
DEFUN(synchronize_cfcard_files_func,
	  synchronize_cfcard_files_cmd,
	  "synchronize cfcard files",
	  "synchronize files on cfcard to other working boards\n"
	  "synchronize version files\n"
	 )
{
    DBusMessage *query = NULL;
    DBusMessage *reply = NULL;
	DBusError err = {0};
	int ret = 0;
	int syn_type = 0;
	int count = 0;
	int ID[MAX_SLOT_NUM] = {0};
	int i = 0;
	char a_returnString[BSD_COMMAND_BUF_LEN] = {0};
    
    count = dcli_bsd_get_slot_ids(dcli_dbus_connection,ID,BSD_TYPE_BOOT_IMG);
    
    if(count != 0)
    {
        for(i = 0; i < count; i++)
        {
            vty_out(vty,"start copying files to slot_%d,please wait...\n",ID[i]);
            ret = dcli_bsd_copy_file_to_board(dcli_dbus_connection,ID[i],"/blk","/blk",0,BSD_TYPE_NORMAL);
            vty_out(vty,"%s", dcli_bsd_get_return_string(ret, a_returnString));
            
        }
    }
    
	return CMD_SUCCESS;			
}




/*****************************************************
** DISCRIPTION:
**          synchronization from master board to others
** INPUT:
**          syn_type    : storage = 1
**                          : wtp_compatible = 2
** OUTPUT:
**          result
** RETURN:
**          
** CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-08-18
*****************************************************/
DEFUN(synchronize_files_func,
	  synchronize_files_cmd,
	  "synchronize wtp",
	  "synchronize files to other working boards\n"
	  "synchronize specific files\n"
	 )
{
    DBusMessage *query = NULL;
    DBusMessage *reply = NULL;
	DBusError err = {0};
	int ret = 0;
	int syn_type = BSD_SYNC_WTP;
	char a_returnString[BSD_COMMAND_BUF_LEN] = {0};
   
    query = dbus_message_new_method_call(BSD_DBUS_BUSNAME,BSD_DBUS_OBJPATH,\
						BSD_DBUS_INTERFACE,BSD_SYNCHRONIZE_FILES_TO_OTHER_BOARDS);
    
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&syn_type,
							 DBUS_TYPE_INVALID);
    
    vty_out(vty, "Copying file, please wait ...\n");
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
	
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&ret,
					DBUS_TYPE_INVALID)) {
		vty_out(vty,"%s", dcli_bsd_get_return_string(ret, a_returnString));
	}
	
	dbus_message_unref(reply);

	return CMD_SUCCESS;
}



/*****************************************************
** DISCRIPTION:
**          set bsd daemon log level
** INPUT:
**          log level
**          log switch
** OUTPUT:
**          result
** RETURN:
**          
** CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2012-02-08
*****************************************************/
DEFUN(set_bsd_daemonlog_debug_open_func,
	  set_bsd_daemonlog_debug_open_cmd,
	  "set bsd daemonlog (default|dbus|bsdinfo|mb|all) debug (open|close)",
	  "bsd config\n"
	  "bsd daemonlog config\n"
	  "bsd daemonlog debug open|close\n"
	 )
{
	int ret;
    unsigned int daemonlogtype;
    unsigned int daemonloglevel;
    int index = 0;
    int indextmp = 0, localid = 1, slot_id = HostSlotId;
	DBusConnection *dbus_connection = dcli_dbus_connection;
    
	if (!strcmp(argv[0],"default")) 
	{
		daemonlogtype = DCLI_BSD_DEFAULT;	
	}
	else if (!strcmp(argv[0],"dbus"))
	{
		daemonlogtype = DCLI_BSD_DBUS;	
	}
	else if (!strcmp(argv[0],"bsdinfo"))
	{
		daemonlogtype = DCLI_BSD_BSDINFO;	
	}
	else if (!strcmp(argv[0],"mb"))
	{
		daemonlogtype = DCLI_BSD_MB;	
	}
	else if (!strcmp(argv[0],"all"))
	{
		daemonlogtype = DCLI_BSD_ALL;	
	}
	else
	{
		vty_out(vty,"<error> input patameter should only be default|dbus|80211|1x|wpa|wapi|all\n");
		return CMD_SUCCESS;
	}
	
	if (!strcmp(argv[1],"open"))
	{
		daemonloglevel = 1;	
	}
	else if (!strcmp(argv[1],"close"))
	{
		daemonloglevel = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter should only be 'open' or 'close'\n");
		return CMD_SUCCESS;
	}

    get_slotid_index(vty, &indextmp, &slot_id, &localid);
    ReInitDbusConnection(&dbus_connection,slot_id,distributFag);

	ret = dcli_set_bsd_daemonlog(index, daemonlogtype, daemonloglevel, dbus_connection, BSD_SET_BSD_DAEMONLOG_LEVEL);

	if(ret == 0) 
	{
		vty_out(vty,"bsd set daemonlog debug %s successfully\n",argv[0]);
	} 
	else 
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
	
	return CMD_SUCCESS;			
}



/*****************************************************
** DISCRIPTION:
**          send files to remote devices
** CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2012-05-25
*****************************************************/

DEFUN(copy_files_between_devices_func,
	  copy_files_between_devices_cmd,
	  "copy SPATH to DPATH (pack|unpack)",
	  "copy files to remote devide\n"
	  "copy /mnt/wtp/wtpcompatible.xml to 10.0.1.2:/mnt/wtp unpack\n"
	  "copy /mnt/wtp/wtpcompatible.xml to 10.0.1.2:/mnt/wtp unpack\n"
	  "copy /mnt/wtp/wtpcompatible.xml to 10.0.1.2:/mnt/wtp unpack\n"
	  "copy /mnt/wtp/wtpcompatible.xml to 10.0.1.2:/mnt/wtp unpack\n"
	 )
{
	int iReturnValue = 0;
	unsigned int uDesAddress = 0;
	char a_cSrcPath[PATH_LEN] = {0};
	char a_cDesPath[PATH_LEN] = {0};
	char a_ipAddr[PATH_LEN] = {0};
	char a_returnString[BSD_COMMAND_BUF_LEN] = {0};
	char sel[BSD_COMMAND_BUF_LEN] = {0};
	int iTarSwitch = 0;
	int iIpAddr = 0;
	int iSocketId = 0;
    int indextmp = 0, localid = 1, slot_id = HostSlotId;
    
    memcpy(a_cSrcPath, argv[0], strlen(argv[0]));
    //printf("src file path = %s\n", (char*)a_cSrcPath);
    
	iReturnValue = dcliParseDesPath((char*)argv[1], (char*)a_ipAddr, (char*)a_cDesPath);
	if(iReturnValue != 0) {
	    vty_out(vty,"input file path is not correct.\n");
	    return CMD_SUCCESS;
	} else {
	    inet_aton((const char *)a_ipAddr, (struct in_addr *)&iIpAddr);
	}
	//printf("ip addr = %s, des file path = %s\n", (char*)a_ipAddr, (char*)a_cDesPath);
    
	if(strcmp(argv[2], "pack") == 0)
	    iTarSwitch = 1;
	else if(strcmp(argv[2], "unpack") == 0)
	    iTarSwitch = 0;

    get_slotid_index(vty, &indextmp, &slot_id, &localid);
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
    
	iReturnValue = dcli_bsd_check_destination_device_information(dcli_dbus_connection,iIpAddr,a_cSrcPath,a_cDesPath,iTarSwitch, BSD_TYPE_NORMAL, &iSocketId);
	
	if(iReturnValue != BSD_SUCCESS) {
	    if(iReturnValue == BSD_ILLEGAL_DESTINATION_FILE_PATH) {
	        /* force to create destination path? */
			vty_out(vty,"Destination path is not exist, forced to create? [yes/no]:\n");
			fscanf(stdin, "%s", sel);
	        while(1) {
				if(!strncasecmp("yes", sel, strlen(sel))) {
				    iReturnValue = dcli_bsd_copy_file_to_device(dcli_dbus_connection,iSocketId,a_cSrcPath,a_cDesPath,iTarSwitch,BSD_TYPE_NORMAL);
					break;
				} else if (!strncasecmp("no", sel, strlen(sel))) {
				    dcli_bsd_close_tcp_socket(dcli_dbus_connection, iSocketId);
				    vty_out(vty, "Operation has been canceled.\n");
					return CMD_SUCCESS;
				} else {
					vty_out(vty,"% Please input 'yes' or 'no'.\n");
					vty_out(vty,"Destination path is not exist, forced to create? [yes/no]:\n");
					memset(sel, 0, PATH_LEN);
					fscanf(stdin, "%s", sel);
				}
			}
	    } else {
	        vty_out(vty,"%s", dcli_bsd_get_return_string(iReturnValue, a_returnString));
	        return CMD_SUCCESS;
	    }
	}
	
	vty_out(vty,"%s", dcli_bsd_get_return_string(iReturnValue, a_returnString));
	
	return CMD_SUCCESS;
}



void dcli_bsd_init(void)
{
//	install_default(CONFIG_NODE);
	install_element(CONFIG_NODE,&copy_files_between_boards_cmd);
	install_element(CONFIG_NODE,&synchronize_files_cmd);
//	install_element(CONFIG_NODE,&synchronize_cfcard_files_cmd);
    install_element(CONFIG_NODE,&set_bsd_daemonlog_debug_open_cmd);
    install_element(CONFIG_NODE,&copy_files_between_devices_cmd);

    install_element(HANSI_NODE,&set_bsd_daemonlog_debug_open_cmd);
    install_element(HANSI_NODE,&copy_files_between_devices_cmd);

    install_element(LOCAL_HANSI_NODE,&set_bsd_daemonlog_debug_open_cmd);
    install_element(LOCAL_HANSI_NODE,&copy_files_between_devices_cmd);
    
	return;
}

#endif
