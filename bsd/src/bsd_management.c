#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <linux/tipc.h>
#include "bsd/bsdpub.h"
#include "bsd.h"
#include "bsd_management.h"
#include "bsd_timerLib.h"
#include "bsd_thread.h"
#include "bsd_tipc.h"
#include "bsd_log.h"
#include "bsd_tcp.h"


#define BM_IOC_MAGIC 	0xEC
#define BM_IOC_ENV_EXCH		_IOWR(BM_IOC_MAGIC,10,boot_env_t)
#define SAVE_BOOT_ENV 	1
#define GET_BOOT_ENV	0

extern char g_rePrintMd5[BSD_PATH_LEN];

typedef struct boot_env
{	
	char name[64];	
	char value[128];	
	int operation;
}boot_env_t;

int tarFlag = 0;
extern int g_iLocalTcpSocketId;


int sor_checkimg(char* imgname)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
	char result_file[64][128];
	char *cmdstr = "sor.sh imgls imgls 180";
	int ret,i,imgnum;
	FILE *fp = 	NULL;
	
	fp = popen( cmdstr, "r" );
	if(fp)
	{
		i=0;
		while(i<64 && fgets( result_file[i], sizeof(result_file[i]), fp ))
			i++;
		imgnum=i;

		ret = pclose(fp);
		
		switch (WEXITSTATUS(ret)) {
			case 0:
				for(i=0;i<imgnum;i++)
				{
					if(!strncasecmp(result_file[i],imgname,strnlen(imgname, 128)))
						return 0;
				}
				return -1;
			default:
				return WEXITSTATUS(ret);
			}
	}
	else
		return -2;
}

int set_boot_img_name(char* imgname)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
	int fd; 
	int retval; 
	boot_env_t	env_args;
	memset(&env_args, 0, sizeof(boot_env_t));
	char *name = "bootfile";
	
	sprintf(env_args.name,name);
	sprintf(env_args.value,imgname);
	env_args.operation = SAVE_BOOT_ENV;

	fd = open("/dev/bm0",0);	
	if(fd < 0)
	{ 	
		return 1;	
	}		
	retval = ioctl(fd,BM_IOC_ENV_EXCH,&env_args);

	if(retval == -1)
	{	
	
		close(fd);
		return 2;	
	}
	close(fd);
	return 0;	
}



/*****************************************************
** DISCRIPTION:
**          check free mem
** INPUT:
**          minMem:     minieum memery should be reserved for system
**          fileLen  :     the size of the file to copy 
** OUTPUT:
**          void
** RETURN:
**          0 :  memery is enough
**          -1: memery is not enough
**CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-08-23
*****************************************************/
static int BSDCheckFreeMem(unsigned int fileLen, unsigned char filePath[PATH_LEN])
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    FILE *fp = NULL;
    char buf[1024] = {0};
    unsigned int freeMem = 0;
    unsigned int minMem = 0;
    unsigned int needMem = fileLen/1024;
    bsd_syslog_debug_debug(BSD_DEFAULT, "filepath = %s\n",(char*)filePath);
    if(0 == memcmp(filePath, "/blk", 4)){
        fp = popen("df -l /blk |grep blk |awk '{ print $4 }'","r");
        //system("sudo umount /blk");
        minMem = CFDISK_MIN_MEM;
    }
    else{
        fp = popen("free | grep + | awk '{ print $4 }'","r");
        minMem = SYSTEM_MIN_MEM;
    }
    if(!fp){
        bsd_syslog_err("failed to get memery information\n");
        return -1;
    }
    
    if(EOF == fread(buf, sizeof(char), sizeof(buf), fp)) {
        bsd_syslog_err("failed to get memery information\n");
    }
    pclose(fp);
    
    if(-1 == parse_int_ID((char*)buf, &freeMem)){
        bsd_syslog_err("Error : failed to parse memery value.\n");
        return -1;
    }
    //bsd_syslog_debug_debug(BSD_DEFAULT, "freemem = %d, minMem = %d, needMem = %d\n",freeMem,minMem,needMem);
    if(freeMem > (minMem + needMem))
        return 0;
    else {
        bsd_syslog_err("freemem = %d, minMem = %d, needMem = %d\n",freeMem,minMem,needMem);
        return -1;
    }
}



/*****************************************************
** DISCRIPTION:
**          check destination path
** INPUT:
**          des file Path
** OUTPUT:
**          void
** RETURN:
**          0 :  legal
**          -1: illegal
**CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2012-2-16
*****************************************************/
static int BSDCheckDesPath(unsigned char filePath[PATH_LEN])
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int iReturnValue = BSD_FILE_DES_PATH_AVA;
    DIR *pSourceDir = NULL;
    char fileDir[PATH_LEN] = {0};
    
    if(BSDGetFileDir((char*)filePath, fileDir) == 0) {
        pSourceDir = opendir(fileDir);
        if(pSourceDir != NULL) {
            closedir(pSourceDir);
            pSourceDir = NULL;
        } else {
            iReturnValue = BSD_FILE_DES_PATH_UNA;
        }
    } else {
        iReturnValue = BSD_FILE_DES_PATH_UNA;
    }
    bsd_syslog_debug_debug(BSD_DEFAULT, "check path ret = %d\n", iReturnValue);
    return iReturnValue;
}



/** 
  * @brief  
  * @param  iFileType   
  * @param  pFileName   
  * @return  
  * @author  zhangshu
  * @date  2012/06/14
  */
static int bsdHandleFileState(const int iFileType, const char *pFilePath)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int iReturnValue = BSD_SUCCESS;
    char fileName[NAME_LEN] = {0};
    char tmpPath[PATH_LEN] = {0};
    char fileDir[PATH_LEN] = {0};
    unsigned int iTmpLen = 0;
    char cmd[BSD_COMMAND_BUF_LEN] = {0};
    if(pFilePath == NULL) {
        bsd_syslog_warning("file name is NULL!\n");
        return iReturnValue;
    }
    bsd_syslog_debug_debug(BSD_DEFAULT, "file type = %d\n", iFileType);
    BSDGetFileDir(pFilePath, fileDir);
	BSDGetFileName(pFilePath, fileName);
	switch(iFileType) {
	    case BSD_TYPE_BOOT_IMG:
	        /* book add for set boot img, 2011-09-15 */
	        if(0 != set_boot_img_name(fileName))
    			bsd_syslog_err("** Error1: failed to set boot img %s in slot %d\n", fileName, HOST_SLOT_NO);
    		else
    			bsd_syslog_debug_debug(BSD_DEFAULT, "set boot img to %s successful\n",fileName);
    	    break;
	    case BSD_TYPE_CORE:
	        bsd_syslog_debug_debug(BSD_DEFAULT, "pFileName = %s\n", pFilePath);
	        bsd_syslog_debug_debug(BSD_DEFAULT, "fileDir = %s\n", fileDir);
	        iTmpLen = strlen("/home/admin");
	        if(memcmp(pFilePath, "/home/admin", iTmpLen) == 0) {
	            memcpy(tmpPath, pFilePath+iTmpLen, strnlen(pFilePath, PATH_LEN) - iTmpLen);
	            bsd_syslog_debug_debug(BSD_DEFAULT, "tmp file path = %s\n", (char*)tmpPath);
	            sprintf(cmd, "sudo cat %s > %s; sudo rm -rf /home/admin/proc;", pFilePath, tmpPath);
	            system(cmd);
	        } else {
	            bsd_syslog_warning("proc file Path is wrong. Please check!!!\n");
	        }
	        break;
	    case BSD_TYPE_PATCH:
	        BSDGetFileName(pFilePath, fileName);
	        sprintf(cmd, "sudo mount /blk; cp %s /mnt/patch/; sudo umount /blk;", pFilePath);
	        system(cmd);
	        break;
	    case BSD_TYPE_WTP_FOLDER:
	        /* add for AXSSZFI-1598 */
	        sprintf(cmd, "sudo mount /blk; mkdir -p /blk/wtp; cp /mnt/wtp/* /blk/wtp/; sudo umount /blk;");
	        system(cmd);
	        break;
	    case BSD_TYPE_WTP:
	        /* add for AXSSZFI-1386 */
	        BSDGetFileName(pFilePath, fileName);
	        sprintf(cmd, "sudo mount /blk; mkdir -p /blk/wtp; cp %s /blk/wtp/; sudo umount /blk;", pFilePath);
	        system(cmd);
	        break;
	    default:
	        break;
	}
    
    return iReturnValue;
}



/*****************************************************
** DISCRIPTION:
**          waiting for tipc message and deal with it
** INPUT:
**          void
** OUTPUT:
**          void
** RETURN:
**          void
**CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-08-10
*****************************************************/
void * bsdTipcManagement()
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
	bsd_pid_write_v2("BSDTipcManagement",HOST_SLOT_NO);
	int fd = -1;
	FILE *fp = NULL;
	struct sockaddr_tipc addr;
	socklen_t alen = sizeof(addr);
	unsigned short last_event_id = 0; /* current & last loop event id */
	char file_dir[PATH_LEN] = {0};
	char file_name[NAME_LEN] = {0};
	char buf[BSD_DATA_LEN+BSD_FILE_HEAD_LEN] = {0}; /* buffer used to save file contents temporary */
	char cmd[BSD_COMMAND_BUF_LEN] = {0};
	unsigned char tmpFilePath[BSD_PATH_LEN]; /* when send to CF/SD card, first save into this tmp Path */
	char *fileData = NULL;
	char *cursor = NULL;    /* used to point effective position of buffer */
	bsd_file_info_t *fileInfo = NULL; 
	fd = HOST_BOARD->tipcfd;
	int result = 0;
	int blk_flag = 0;
	int slotid = 0;
	int err_flag = 0; /* used to mark receiving error during one event */
	char md5Value[BSD_COMMAND_BUF_LEN] = {0};
	int bak_config_flag = 0;
	int active_master_slot_id = 0;
	int slot_id = 0;
	char temp_buf[100] = {0};
	char temp_buf2[100] = {0};	
	int product_type = 0;
	while(1) {
		if (0 >= recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&addr, &alen)){
			perror("Server: unexpected message");
		}
		
		fileInfo = (bsd_file_info_t *)buf;
        slotid = fileInfo->slot_no;
        //bsd_syslog_debug_debug(BSD_DEFAULT, "recv file state = %d\n", fileInfo->file_head.file_state);
        /* 2012-4-12 */
        if(fileInfo->file_head.event_id != last_event_id) {
            if(fileData != NULL) {
                free(fileData);
                fileData = NULL;
                cursor = NULL;
            }
            err_flag = 0; /* reset err_flag for new event */
            last_event_id = fileInfo->file_head.event_id; /* record current event id */
            bsd_syslog_debug_debug(BSD_DEFAULT, "recv event_id = %d\n",fileInfo->file_head.event_id);
        }
        
        /* deal with messages */
        if((fileInfo->file_head.file_state == BSD_FILE_FINISH)&&(BSD_BOARD[slotid]->state == BSD_FILE_FINISH)){
            bsd_syslog_debug_debug(BSD_DEFAULT, "recv msg type is BSD_FILE_FINISH\n");
            BsdThreadMutexLock(&fileStateMutex[slotid]);
            memset(g_rePrintMd5, 0, BSD_PATH_LEN);
		    memcpy(g_rePrintMd5, fileInfo->file_head.md5Result, BSD_PATH_LEN);
            BSD_BOARD[slotid]->state = BSD_FILE_UNKNOWN;
            BsdSignalThreadCondition(&fileStateCondition[slotid]);
            BsdThreadMutexUnlock(&fileStateMutex[slotid]);
            bsd_syslog_debug_debug(BSD_DEFAULT, "singal condition sendCondition\n");
            continue;
        } else if(fileInfo->file_head.file_state == BSD_FILE_MEMERY_NOT_ENOUGH) {
		    bsd_syslog_debug_debug(BSD_DEFAULT, "@@@ recv msg type is BSD_FILE_MEMERY_NOT_ENOUGH\n");
		    BsdThreadMutexLock(&fileStateMutex[slotid]);
		    BSD_BOARD[slotid]->state = BSD_FILE_MEMERY_NOT_ENOUGH;
            BsdSignalThreadCondition(&fileStateCondition[slotid]);
            BsdThreadMutexUnlock(&fileStateMutex[slotid]);
            bsd_syslog_debug_debug(BSD_DEFAULT, "singal condition checkCondition\n");
		    continue;
		} else if(fileInfo->file_head.file_state == BSD_FILE_MEMERY_OK){
		    bsd_syslog_debug_debug(BSD_DEFAULT, "recv msg type is BSD_FILE_MEMERY_OK\n");
		    BsdThreadMutexLock(&fileStateMutex[slotid]);
		    BSD_BOARD[slotid]->state = BSD_FILE_MEMERY_OK;
            BsdSignalThreadCondition(&fileStateCondition[slotid]);
            BsdThreadMutexUnlock(&fileStateMutex[slotid]);
            bsd_syslog_debug_debug(BSD_DEFAULT, "singal condition checkCondition\n");
		    continue;
		} else if(fileInfo->file_head.file_state == BSD_FILE_DES_PATH_CHECK){
		    system("sudo mount /blk");
		    result = BSDCheckDesPath(fileInfo->file_head.uchFileName);
		    system("sudo umount /blk");
		    BSDSendNotify(slotid, result, last_event_id, md5Value);
		    continue;
		} else if(fileInfo->file_head.file_state == BSD_FILE_DES_PATH_UNA) {
		    BsdThreadMutexLock(&fileStateMutex[slotid]);
		    BSD_BOARD[slotid]->state = BSD_FILE_DES_PATH_UNA;
            BsdSignalThreadCondition(&fileStateCondition[slotid]);
            BsdThreadMutexUnlock(&fileStateMutex[slotid]);
		    continue;
		} else if(fileInfo->file_head.file_state == BSD_FILE_DES_PATH_AVA) {
		    BsdThreadMutexLock(&fileStateMutex[slotid]);
		    BSD_BOARD[slotid]->state = BSD_FILE_DES_PATH_AVA;
            BsdSignalThreadCondition(&fileStateCondition[slotid]);
            BsdThreadMutexUnlock(&fileStateMutex[slotid]);
		    continue;
		} else if(fileInfo->file_head.file_state == BSD_FILE_MEMERY_CHECK) {
		    system("sudo mount /blk");
		    if(0 == BSDCheckFreeMem(fileInfo->file_head.file_total_len, fileInfo->file_head.uchFileName)){
		        result = BSD_FILE_MEMERY_OK;
		    }
		    else{
		        result = BSD_FILE_MEMERY_NOT_ENOUGH;
		    }
		    system("sudo umount /blk");
		    BSDSendNotify(slotid, result, last_event_id, md5Value);
		    continue;
		} else if(fileInfo->file_head.file_state == BSD_FILE_FAILURE) {
		    BsdThreadMutexLock(&fileStateMutex[slotid]);
		    memset(g_rePrintMd5, 0, BSD_PATH_LEN);
		    memcpy(g_rePrintMd5, fileInfo->file_head.md5Result, BSD_PATH_LEN);
		    BSD_BOARD[slotid]->state = BSD_FILE_FAILURE;
            BsdSignalThreadCondition(&fileStateCondition[slotid]);
            BsdThreadMutexUnlock(&fileStateMutex[slotid]);
		    continue;
		} else if(fileInfo->file_head.file_state == BSD_FILE_UNKNOWN) {
		    continue;
		} else if(((fileInfo->file_head.file_state == BSD_FILE_NORMAL) && (!err_flag)) 
		    || (fileInfo->file_head.file_state == BSD_FILE_LAST)) {
		    if(fileData == NULL) {
    		    fileData = (char*)malloc(fileInfo->file_head.file_total_len + 1);
    		    memset(fileData, 0, fileInfo->file_head.file_total_len+1);
    		    cursor = fileData;
    		}
            /* 2012-4-11 */
            if((cursor != NULL) && (fileInfo->file_head.send_len <= BSD_DATA_LEN)) {
                memcpy(cursor, (char*)fileInfo->uchData, fileInfo->file_head.send_len);
    		    cursor += fileInfo->file_head.send_len;
            } else {
                bsd_syslog_err("Error: failed to copy data from buffer.\n");
                free(fileData);
    		    fileData = NULL;
    		    cursor = NULL;
    		    err_flag = 1;
    		    continue;
            }
		}
		if(!strncmp((char *)fileInfo->file_head.uchFileName,"/var/run/config_bak",strlen("/var/run/config_bak")))
			bak_config_flag = 1;
		
		memset(temp_buf2,0,sizeof(temp_buf2));		//get active_master_slot_id by houxx 2013819
		strcpy(temp_buf,"/dbm/local_board/is_active_master");
		fp = fopen(temp_buf, "r");
		if (fp == NULL){
			bsd_syslog_err("###    Failed to open file\n");
			free(fileData);
			fileData = NULL;
			cursor = NULL;
			system("sudo umount /blk");
			BSDSendNotify(slotid, BSD_FILE_FAILURE, last_event_id, md5Value);
			continue;
		}
		if(EOF == fscanf(fp, "%d", &active_master_slot_id)) {
			bsd_syslog_err("###    Failed to get file\n");
			free(fileData);
			fileData = NULL;
			cursor = NULL;
			system("sudo umount /blk");
			BSDSendNotify(slotid, BSD_FILE_FAILURE, last_event_id, md5Value);
			continue;
		}
		fclose(fp);
		
		memset(temp_buf2,0,sizeof(temp_buf2));	//get slot_id by houxx 2013819
		strcpy(temp_buf2,"/dbm/local_board/slot_id");
		fp = fopen(temp_buf2, "r");
		if (fp == NULL){
			bsd_syslog_err("###    Failed to open file\n");
			free(fileData);
			fileData = NULL;
			cursor = NULL;
			system("sudo umount /blk");
			BSDSendNotify(slotid, BSD_FILE_FAILURE, last_event_id, md5Value);
			continue;
		}
		if(EOF == fscanf(fp, "%d", &slot_id)) {
			bsd_syslog_err("###    Failed to get file\n");
			free(fileData);
			fileData = NULL;
			cursor = NULL;
			system("sudo umount /blk");
			BSDSendNotify(slotid, BSD_FILE_FAILURE, last_event_id, md5Value);
			continue;
		}
		fclose(fp);

		memset(temp_buf2,0,sizeof(temp_buf2));	//get product_type by houxx 2013819
		strcpy(temp_buf2,"/dbm/product/product_type");
		fp = fopen(temp_buf2, "r");
		if (fp == NULL){
			bsd_syslog_err("###    Failed to open file\n");
			free(fileData);
			fileData = NULL;
			cursor = NULL;
			system("sudo umount /blk");
			BSDSendNotify(slotid, BSD_FILE_FAILURE, last_event_id, md5Value);
			continue;
		}
		if(EOF == fscanf(fp, "%d", &product_type)) {
			bsd_syslog_err("###    Failed to get product_type\n");
			free(fileData);
			fileData = NULL;
			cursor = NULL;
			system("sudo umount /blk");
			BSDSendNotify(slotid, BSD_FILE_FAILURE, last_event_id, md5Value);
			continue;
		}
		fclose(fp);
		
		bsd_syslog_err("###    in func %s,line %d,slot_id is %d,active_master_slot_id is %d back is %d\n",__func__,__LINE__,slot_id,active_master_slot_id,bak_config_flag);
		if(fileInfo->file_head.file_state == BSD_FILE_LAST) {
		    blk_flag = 0;
		    memset(tmpFilePath, 0, BSD_PATH_LEN);
		    system("sudo mount /blk");
		    memset(file_dir, 0, PATH_LEN);
		    memset(file_name, 0, NAME_LEN);
		    bsd_syslog_debug_debug(BSD_DEFAULT, "file path = %s\n",(char *)fileInfo->file_head.uchFileName);
		    BSDGetFileDir((const char *)fileInfo->file_head.uchFileName, file_dir);
		    BSDGetFileName((const char *)fileInfo->file_head.uchFileName, file_name);

            /* if copy to /blk, first copy to memery then mv to /blk, book add 2012-2-8 */
            if((fileInfo->file_head.file_type == BSD_TYPE_BOOT_IMG) 
                || (fileInfo->file_head.file_type == BSD_TYPE_PATCH) 
                || (fileInfo->file_head.file_type == BSD_TYPE_BLK)) {
                strncpy((char*)tmpFilePath, "/mnt/", 5);
                strncat((char*)tmpFilePath, (char*)file_name, strnlen((char*)file_name, NAME_LEN));
                bsd_syslog_debug_debug(BSD_DEFAULT, "tmpFilePath = %s\n", (char*)tmpFilePath);
                if(0 == BSDCheckFreeMem(fileInfo->file_head.file_total_len, tmpFilePath)) {
    		        blk_flag = 1;
    		    }
            }
            bsd_syslog_debug_debug(BSD_DEFAULT, "blk_flag = %d\n", blk_flag);
            if(!blk_flag) {
                memset(tmpFilePath, 0, BSD_PATH_LEN);
                strncpy((char*)tmpFilePath, (char*)fileInfo->file_head.uchFileName, BSD_PATH_LEN);
            }

		    /* mkdir for file */
		    if(bsdCreateDir(file_dir) == -1)
		        bsd_syslog_debug_debug(BSD_DEFAULT, "make dir %s failed.\n", (char*)file_dir);
		    //printf("make dir %s\n",(char *)file_dir);
		    FILE * fp = NULL;
            fp = fopen((char*)tmpFilePath, "wb");
            //printf("file path = %s\n",(char*)fileInfo->file_head.uchFileName);
            if((fp != NULL) && (fileData != NULL)) {
                bsd_syslog_debug_debug(BSD_DEFAULT, "open file successful\n");
                fwrite(fileData, fileInfo->file_head.file_total_len, 1, fp);
                fclose(fp);
                fp = NULL;
                bsd_syslog_debug_debug(BSD_DEFAULT, "write file successful\n");
    		    if(fileInfo->file_head.tar_flag) {
    		        memset(cmd, 0, BSD_COMMAND_BUF_LEN);
    		        sprintf(cmd, "cd %s; sudo tar -xvf %s; sudo rm %s",\
    		        (char*)file_dir, file_name, file_name);
    		        bsd_syslog_debug_debug(BSD_DEFAULT, "%s\n",(char*)cmd);
    		        system(cmd);
    		    }
    		    free(fileData);
    		    fileData = NULL;
    		    cursor = NULL;
                
                if(blk_flag) {
                    if((fileInfo->file_head.file_type == BSD_TYPE_BOOT_IMG) || (fileInfo->file_head.file_type == BSD_TYPE_BLK)) {
                        memset(cmd, 0, BSD_COMMAND_BUF_LEN);
                        sprintf(cmd,"cd /mnt; sudo sor.sh cp %s 120; sudo rm %s;",(char*)file_name, (char*)file_name);
                        system(cmd);
                    } else if(fileInfo->file_head.file_type == BSD_TYPE_PATCH) {
                        memset(cmd, 0, BSD_COMMAND_BUF_LEN);
                        sprintf(cmd,"sudo cp %s %s; sudo rm %s;",(char*)tmpFilePath, (char*)fileInfo->file_head.uchFileName, (char*)tmpFilePath);
                        system(cmd);
                    }
                    bsd_syslog_debug_debug(BSD_DEFAULT,"copy file from /mnt to /blk\n");    
                }
    		    
    		    bsd_syslog_debug_debug(BSD_DEFAULT,"save file successful\n");
				bsd_syslog_debug_debug(BSD_DEFAULT,"type = %d\n",fileInfo->file_head.file_type);

				/* calculate md5 value */
				if((fileInfo->file_head.file_type == BSD_TYPE_BOOT_IMG) 
				    || (fileInfo->file_head.file_type == BSD_TYPE_PATCH)
				    || (fileInfo->file_head.file_type == BSD_TYPE_WTP)
				    || ((fileInfo->file_head.file_type == BSD_TYPE_BLK))) {
				    memset(md5Value, 0, BSD_COMMAND_BUF_LEN);
				    if(BSDCalculateMd5((const char *)fileInfo->file_head.uchFileName, md5Value) != BSD_SUCCESS) {
				        bsd_syslog_err("Error : calculate md5 value for file %s failed.", \
				            (char*)fileInfo->file_head.uchFileName);
				        BSDSendNotify(slotid, BSD_FILE_FAILURE, last_event_id, md5Value);
				        continue;
				    } 
				    if(memcmp(fileInfo->file_head.md5Result, md5Value, strnlen(md5Value, BSD_PATH_LEN)) != 0) {
				        bsd_syslog_err("Error : compare md5 value failed, src %s, dst %s.", \
				            (char*)fileInfo->file_head.md5Result, (char*)md5Value);
				        BSDSendNotify(slotid, BSD_FILE_FAILURE, last_event_id, md5Value);
				        continue;    
				    }
				}
				bsdHandleFileState(fileInfo->file_head.file_type, (const char *)fileInfo->file_head.uchFileName);
				BSDSendNotify(slotid, BSD_FILE_FINISH, last_event_id, md5Value);
				system("sudo umount /blk");
				if(product_type == 4 || product_type == 5 || product_type == 7 || product_type == 6)
				{
					if(bak_config_flag == 1 && active_master_slot_id == 1)
					{
						bsd_syslog_err("###    in func %s,line %d\n",__func__,__LINE__);
						bsd_get_hansiprofile_notity_hmd(fileInfo);
					}
				}
            }
            else {
                //printf("Failed to open file\n");
                bsd_syslog_err("###    Failed to open file\n");
                free(fileData);
    		    fileData = NULL;
    		    cursor = NULL;
    		    system("sudo umount /blk");
    		    BSDSendNotify(slotid, BSD_FILE_FAILURE, last_event_id, md5Value);
                continue;
            }
		}
	}
	return 0;
}



/** 
  * @brief  receive tcp messages and manage
  * @return  void
  * @author  zhangshu
  * @date  2012/05/24
  */
void * bsdTcpManagement()
{
    bsd_syslog_debug_debug(BSD_BSDINFO, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int i = 0;
    int maxSocketId = 0;
    fd_set tcpFdSet;
    struct timeval selectTimeout;
    struct STcpClientSockets clientSockets;
    memset(&clientSockets, 0, sizeof(struct STcpClientSockets));
    struct STcpSocketNode *clientSocketNode = NULL;
    
    while(1) {
        /* reset resource */
        FD_ZERO(&tcpFdSet);
        maxSocketId = 0;
        memset(&selectTimeout, 0, sizeof(struct timeval));
        clientSocketNode = NULL;
        
        FD_SET(g_iLocalTcpSocketId, &tcpFdSet);
        maxSocketId = g_iLocalTcpSocketId;
        clientSocketNode = clientSockets.clientSocketList;
        for(i = 0; (i < clientSockets.iClientSocketCount) && (clientSocketNode != NULL); i++) {
    		FD_SET(clientSocketNode->iSocketId, &tcpFdSet);
    		if(clientSocketNode->iSocketId > maxSocketId) 
    		    maxSocketId = clientSocketNode->iSocketId;
    		if(clientSocketNode->next != NULL)
    			clientSocketNode = clientSocketNode->next;
    	    else
    	        break;
    	}
    	
    	selectTimeout.tv_sec = 2;
    	selectTimeout.tv_usec = 0;
    	
    	while(select(maxSocketId+1, &tcpFdSet, NULL, NULL, &selectTimeout) < 0) {
    		if(errno != EINTR) 
                bsd_syslog_err("Error : select socket set failed.\n");
    	}
    	
	    if(FD_ISSET(g_iLocalTcpSocketId, &tcpFdSet)) {
			/* this is tcp connect message */
			bsd_syslog_debug_debug(BSD_DEFAULT, "g_iLocalTcpSocketId = %d\n", g_iLocalTcpSocketId);
			bsdAcceptTcpConnection(&clientSockets);
		} else {
		    clientSocketNode = clientSockets.clientSocketList;
		    for(i = 0; (i < clientSockets.iClientSocketCount) && (clientSocketNode != NULL); i++,clientSocketNode = clientSocketNode->next) {
		        if(FD_ISSET(clientSocketNode->iSocketId, &tcpFdSet)) {
		            /* this is tcp send message */
		            bsdReceiveTcpMessage(&clientSockets, clientSocketNode);
		        }
		    }
    	}
    }
    
    return 0;
}
