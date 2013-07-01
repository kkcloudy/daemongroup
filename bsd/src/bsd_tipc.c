#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <ftw.h>
#include <fcntl.h>
#include <limits.h>
#include <dirent.h>
#include <linux/tipc.h>
//#include "board/board_define.h"
#include "bsd/bsdpub.h"
#include "bsd.h"
#include "bsd_log.h"
#include "bsd_management.h"
#include "bsd_timerLib.h"
#include "bsd_thread.h"
#include "bsd_tipc.h"

enum board_function_type{
	UNKNOWN_BOARD = 0,
	MASTER_BOARD = 0x1,		/*master board*/
	AC_BOARD = 0x2,			/*wireless business board*/
	SWITCH_BOARD = 0x4,		/*switch board*/
	BASE_BOARD = 0x8,		/*Bas board*/
	NAT_BOARD = 0x10		/*NAT board*/
};

extern unsigned short g_unEventId;


/*****************************************************
** DISCRIPTION:
**          bsd init tipc server
** INPUT:
**          slot id
** OUTPUT:
**          null
** RETURN:
**          socket fd
** CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-08-11
*****************************************************/
int BSDServerTipcInit(unsigned int slotid)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
	int fd;
	struct sockaddr_tipc server_addr;
	server_addr.family = AF_TIPC;
	server_addr.addrtype = TIPC_ADDR_NAMESEQ;
	server_addr.addr.nameseq.type = BSD_TIPC_SERVER_TYPE;
	server_addr.addr.nameseq.lower = BSD_SERVER_BASE_INST+slotid;
	server_addr.addr.nameseq.upper = BSD_SERVER_BASE_INST+slotid;
	server_addr.scope = TIPC_ZONE_SCOPE;
	fd = socket(AF_TIPC, SOCK_RDM, 0);
	if(fd < 0) {
	    bsd_syslog_err("Server: failed to create socket for host\n");
	} else if (0 != bind(fd, (struct sockaddr *)&server_addr, sizeof(server_addr))){
		bsd_syslog_err("Server: failed to bind port name\n");
		//exit(1);
	}
    if(HOST_BOARD == NULL){
		HOST_BOARD = (struct Bsd_Board_Info *)malloc(sizeof(struct Bsd_Board_Info));
	}
	memset(HOST_BOARD, 0, sizeof(struct Bsd_Board_Info));
	HOST_BOARD->tipcaddr.family = server_addr.family;
	HOST_BOARD->tipcaddr.addrtype = server_addr.addrtype;
	HOST_BOARD->tipcaddr.addr.nameseq.type  = server_addr.addr.nameseq.type;
	HOST_BOARD->tipcaddr.addr.nameseq.lower = server_addr.addr.nameseq.lower;
	HOST_BOARD->tipcaddr.addr.nameseq.upper = server_addr.addr.nameseq.upper;
	HOST_BOARD->slot_no = slotid;
	HOST_BOARD->tipcfd = fd;
	HOST_BOARD->state = BSD_FILE_UNKNOWN;
	
	return fd;
	//exit(0);
}



/*****************************************************
** DISCRIPTION:
**          Init tipc clients
** INPUT:
**          void
** OUTPUT:
**          void
** RETURN:
**          void
** CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-08-11
*****************************************************/
void BSDClientTipcInit()
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int i = 0;
    for(i = 0; i < MAX_SLOT_NUM; i++){
    
        struct sockaddr_tipc server_addr;
    	int sd = socket (AF_TIPC, SOCK_RDM,0);
    	server_addr.family = AF_TIPC;
    	server_addr.addrtype = TIPC_ADDR_NAME;
    	server_addr.addr.name.name.type = BSD_TIPC_SERVER_TYPE;
    	server_addr.addr.name.name.instance = BSD_SERVER_BASE_INST+i;
    	server_addr.addr.name.domain = 0;

        if(BSD_BOARD[i] == NULL){
			BSD_BOARD[i] = (struct Bsd_Board_Info *)malloc(sizeof(struct Bsd_Board_Info));
		}
		memset(BSD_BOARD[i], 0, sizeof(struct Bsd_Board_Info));
    	BSD_BOARD[i]->tipcaddr.family = server_addr.family;
    	BSD_BOARD[i]->tipcaddr.addrtype = server_addr.addrtype;
    	BSD_BOARD[i]->tipcaddr.addr.name.name.type = server_addr.addr.name.name.type;
    	BSD_BOARD[i]->tipcaddr.addr.name.name.instance = server_addr.addr.name.name.instance;
    	BSD_BOARD[i]->tipcaddr.addr.name.domain = server_addr.addr.name.domain;
    	BSD_BOARD[i]->tipcfd = sd;
    	BSD_BOARD[i]->slot_no = i;
    	BSD_BOARD[i]->state = BSD_FILE_UNKNOWN;
        
    	bsd_syslog_info("****** TIPC client hello world program started ******\n\n");    
    }
    
	return ;
	//exit(0);
}



/*****************************************************
** DISCRIPTION:
**          check whether the destination ticp is active
** INPUT:
**          tipc name
**          wait
** OUTPUT:
**          void
** RETURN:
**          0   yes
**          1   no
** CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-08-11
*****************************************************/
int wait_for_server(struct tipc_name* name,int wait)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    struct sockaddr_tipc topsrv;
    struct tipc_subscr subscr = {{name->type,name->instance,name->instance},wait,TIPC_SUB_SERVICE,{}};
    struct tipc_event event;

    int sd = socket (AF_TIPC, SOCK_SEQPACKET,0);
    if(sd < 0) return 1;

    memset(&topsrv,0,sizeof(topsrv));
    topsrv.family = AF_TIPC;
    topsrv.addrtype = TIPC_ADDR_NAME;
    topsrv.addr.name.name.type = TIPC_TOP_SRV;
    topsrv.addr.name.name.instance = TIPC_TOP_SRV;

    if (0 > connect(sd,(struct sockaddr*)&topsrv,sizeof(topsrv))){
            bsd_syslog_err("failed to connect to topology server");
            close(sd);
            return 1;
    }

    if (send(sd,&subscr,sizeof(subscr),0) != sizeof(subscr)){
            bsd_syslog_err("failed to send subscription");
            close(sd);
            return 1;
    }

    if (recv(sd,&event,sizeof(event),0) != sizeof(event)){
            bsd_syslog_err("Failed to receive event");
            close(sd);
            return 1;
    }

    if (event.event != TIPC_PUBLISHED){
            bsd_syslog_err("Server %u,%u not published within %u [s]\n",
                   name->type,name->instance,wait/1000);
            close(sd);
            return 1;
    }
    bsd_syslog_debug_debug(BSD_DEFAULT, "\n****** close(sd)******\n");
    close(sd);
	return 0;
}


/*****************************************************
** DISCRIPTION:
**          add msg to msgQueue
** INPUT:
**          target slot ID
**          source dir
**          target dir
** OUTPUT:
**          void
** RETURN:
**          void
** CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-08-22
*****************************************************/
int BSDAddMessageQueue(const unsigned int slotid, const char *src_path, const char *des_path, const int op)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    struct BsdMsgQ msgq;
    msgq.mqid = slotid;
	msgq.mqinfo.op = op; //book add, 2011-09-15
    msgq.mqinfo.tar_slot_id = slotid;
    memset(msgq.mqinfo.src_path, 0, PATH_LEN);
    memcpy(msgq.mqinfo.src_path, src_path, PATH_LEN);
    memset(msgq.mqinfo.tar_path, 0, PATH_LEN);
    memcpy(msgq.mqinfo.tar_path, des_path, PATH_LEN);
    
    if (msgsnd(BSDMsgqID, (struct BsdMsgQ *)&msgq, sizeof(msgq.mqinfo), 0) == -1)
    {
	    perror("msgqsnd");
	    bsd_syslog_err("Add message to msgq error.");
	    return BSD_ADD_TO_MESSAGE_QUEUE_ERROR;
    }
    return BSD_SUCCESS;
}




/*****************************************************
** DISCRIPTION:
**          copy dir between boards
** INPUT:
**          target slot ID
**          source dir
**          target dir
** OUTPUT:
**          void
** RETURN:
**          0   yes
**          -1   no
** CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-08-15
*****************************************************/
int BSDSendDir(const unsigned int slotid, const char *source_dir, const char *target_dir, const int op)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    //printf("slotid3 = %d\n", slotid);
    //printf("send dir %s\n",source_dir);
    int ret = 0;
    DIR *source = NULL;
    struct dirent *ent=NULL;
    char name1[PATH_LEN],name2[PATH_LEN];
    bsd_syslog_debug_debug(BSD_DEFAULT, "source_dir = %s\n",source_dir);
    system("sudo mount /blk");
    source = opendir(source_dir);
    if(source == NULL){
    /* copy files */
        if(0 == access(source_dir, F_OK)){
            //printf("file exists\n");
            ret = BSDAddMessageQueue(slotid, source_dir, target_dir, op); 
        }
        else{
            bsd_syslog_err("source file doesn't exist\n");
            ret = BSD_ILLEGAL_SOURCE_FILE_PATH;
        }
    }
    else{
        while((ent = readdir(source)) != NULL)
        { 
            if( strcmp(ent->d_name, "..")!=0 && strcmp(ent->d_name, ".")!=0)
            {               
                strcpy(name1, "\0");
                strcat(name1, source_dir);
                strcat(name1, "/");
                strcat(name1, ent->d_name);
                strcpy(name2, "\0");
                strcat(name2, target_dir);
                strcat(name2, "/"); 
                strcat(name2, ent->d_name);
                bsd_syslog_debug_debug(BSD_DEFAULT, "name1 = %s\n",name1);
                bsd_syslog_debug_debug(BSD_DEFAULT, "name2 = %s\n",name2);
                if(ent->d_type == DIR_TPYE)
                    ret = BSDSendDir(slotid, name1, name2, op);
                if(ent->d_type == FILE_TYPE)
                    ret = BSDAddMessageQueue(slotid, name1, name2, op);
            }
        }
        closedir(source);
    }
    system("sudo umount /blk");
    return ret;
}



/*****************************************************
** DISCRIPTION:
**          send memery check request
** INPUT:
**          slot ID
**          originate file path
**          destination file path
** OUTPUT:
**          void
** RETURN:
**          0   successful
**          other fail
** CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-08-23
*****************************************************/
int BSDSendMemeryCheckRequest(const unsigned int slotid, const char *src_path, const char *des_path, unsigned short event_id)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    unsigned int file_len = 0;
    int iReturnValue = BSD_SUCCESS;
    FILE *fp = NULL;
	int sd = BSD_BOARD[slotid]->tipcfd;
	bsd_file_info_t fileInfo;
	memset(&fileInfo, 0, sizeof(bsd_file_info_t));
	fileInfo.slot_no = HOST_SLOT_NO;
	fileInfo.file_head.file_state = BSD_FILE_MEMERY_CHECK;
	fileInfo.file_head.event_id = event_id;
	system("sudo mount /blk");
	fp = fopen(src_path, "rb");
    if(!fp){
        bsd_syslog_err("failed to open file %s\n",src_path);
    	//bsd_syslog_debug_debug(BSD_DEFAULT, "open file %s to syn failed\n", src_path);
	    iReturnValue = BSD_ILLEGAL_SOURCE_FILE_PATH;
    } else {
        //printf("111\n");
    	fseek(fp, 0, SEEK_END);
        file_len = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        //printf("file_len = %u\n", file_len);
        fclose(fp);
    	fp = NULL;
        fileInfo.file_head.file_total_len = file_len;
        memcpy(fileInfo.file_head.uchFileName, des_path, PATH_LEN);

        if(0 != wait_for_server(&(BSD_BOARD[slotid]->tipcaddr.addr.name.name),10000)){
    		bsd_syslog_err("******server not catch*\n\n");
    		//printf("******server not catch*\n");
    		iReturnValue = BSD_SERVER_NOT_CATCH;
    	}
    	else{
    	    bsd_syslog_debug_debug(BSD_DEFAULT, "******server ok*\n\n");
    		//printf("******server ok*\n");
    		if (0 > sendto(sd, (char*)&fileInfo, sizeof(fileInfo), 0, (struct sockaddr*)&BSD_BOARD[slotid]->tipcaddr, sizeof(struct sockaddr))){
                //printf("d\n");
                bsd_syslog_err(" send message to slot %d fail.", slotid);
        		iReturnValue = BSD_SEND_MESSAGE_ERROR;
            } else {
                bsd_syslog_debug_debug(BSD_DEFAULT,"send memery check request to slot%d ok\n",slotid);
            }
    	}
    }
	system("sudo umount /blk");
    return iReturnValue;
}



/*****************************************************
** DISCRIPTION:
**          send specific file to destination tipc
** INPUT:
**          destination slot ID
**          originate file path
**          destination file path
** OUTPUT:
**          void
** RETURN:
**          0   successful
**          other fail
** CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-08-11
*****************************************************/
int BSDSendFile(const unsigned int slotid, const char *src_path, const char *des_path, int op, char *md5Result)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
	unsigned int file_len = 0;
	unsigned int send_len = 0;
	int remaind_len = 0;
	struct timespec timeout;
	char cmd[BSD_COMMAND_BUF_LEN] = {0};
	int ret = BSD_SUCCESS;
	FILE *fp = NULL;
	int sd = BSD_BOARD[slotid]->tipcfd;
	bsd_file_info_t fileInfo;
	memset(&fileInfo, 0, sizeof(bsd_file_info_t));
    system("sudo mount /blk");
    /* 2012-4-12 */
    fileInfo.file_head.event_id = g_unEventId;
    bsd_syslog_debug_debug(BSD_DEFAULT, "send event_id = %d\n",fileInfo.file_head.event_id);

    if((op == BSD_TYPE_COMPRESS) || (op == BSD_TYPE_WTP_FOLDER)) {
	    fileInfo.file_head.tar_flag = 1;
	}
	fileInfo.slot_no = HOST_SLOT_NO;
    
	/* book add, 2011-09-15 */
	fileInfo.file_head.file_type = op;
	bsd_syslog_debug_debug(BSD_DEFAULT, "file_type = %d\n", fileInfo.file_head.file_type);
	
    //printf("tar_path = %s\n",(char *)newDesPath);
    bsd_syslog_debug_debug(BSD_DEFAULT, "file_path = %s\n", (char*)src_path);
	fp = fopen((char*)src_path, "rb");
    if(!fp){
        bsd_syslog_err("failed to open file %s\n",(char*)src_path);
    	//bsd_syslog_debug_debug(BSD_DEFAULT, "open file %s to syn failed\n", src_path);
	    ret = BSD_ILLEGAL_SOURCE_FILE_PATH;
    } else {
        //bsd_syslog_debug_debug(BSD_DEFAULT, "###    Open file successful\n");
        //printf("111\n");
    	fseek(fp, 0, SEEK_END);
        file_len = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        //printf("file_len = %u\n", file_len);
        fileInfo.file_head.file_total_len = file_len;
        fileInfo.file_head.file_already_len = 0;
        remaind_len = fileInfo.file_head.file_total_len;
        memcpy(fileInfo.file_head.uchFileName, des_path, BSD_PATH_LEN);
        /* book add, 2012-8-4 */
        memcpy(fileInfo.file_head.md5Result, md5Result, strnlen(md5Result, BSD_PATH_LEN));
            
        //bsd_syslog_info("###    File length  :  %llu\n",file_len);
        bsd_syslog_debug_debug(BSD_DEFAULT, "fileInfo.file_head.md5Result = %s", (char*)fileInfo.file_head.md5Result);
        
        if(0 != wait_for_server(&(BSD_BOARD[slotid]->tipcaddr.addr.name.name),10000)) {
    		bsd_syslog_err("******server not catch*\n\n");
    		ret = BSD_SERVER_NOT_CATCH;
    	} else {
    		bsd_syslog_debug_debug(BSD_DEFAULT, "******server ok*\n\n");
    		//printf("222\n");
            while(1) {
        		memset(fileInfo.uchData, 0, BSD_DATA_LEN);
        	    send_len = fread(fileInfo.uchData, sizeof(char), BSD_DATA_LEN-1, fp); /*read file data*/
        	    fileInfo.file_head.send_len = send_len;
                fileInfo.file_head.file_already_len += send_len;
                remaind_len -= send_len;
                if(remaind_len > 0){
            	    fileInfo.file_head.file_state = BSD_FILE_NORMAL;            /*no end of the file*/
                }
                else{
            	    fileInfo.file_head.file_state = BSD_FILE_LAST;            /*end of the file*/
                }
                //bsd_syslog_debug_debug(BSD_DEFAULT, "file state = %d\n", fileInfo.file_head.file_state);
                //printf("c\n");
                if (0 > sendto(sd, (char*)&fileInfo, sizeof(fileInfo), 0, (struct sockaddr*)&BSD_BOARD[slotid]->tipcaddr, sizeof(struct sockaddr))){
                    //printf("d\n");
                    //fclose(fp);
        			//fp = NULL;
        			//printf("Send file(%s) len(%u) Failed.\n", src_path, file_len);
        			bsd_syslog_err("Send file(%s) len(%u) Failed.\n", (char*)src_path, file_len);
        			ret = BSD_SEND_MESSAGE_ERROR;
        			break;
        	    } else {
            	    //bsd_syslog_debug_debug(BSD_DEFAULT, "send fragment to slot%d.\n", slotid);
            	    usleep(100);
            		//printf("e\n");
            	    if(fileInfo.file_head.file_state == BSD_FILE_LAST) {
            			bsd_syslog_debug_debug(BSD_DEFAULT, "\tSend file(%s) len(%u) successed.\n", (char*)src_path, file_len);
                        //fclose(fp);
                        //bsd_syslog_info("###    Send file over\n");
                        //fp = NULL;
                        if((op == BSD_TYPE_COMPRESS) || (op == BSD_TYPE_WTP_FOLDER)) {
                            memset(cmd, 0, BSD_COMMAND_BUF_LEN);
                            sprintf(cmd, "sudo rm %s;", (char*)src_path);
                            system(cmd);
                        }

                        if(op != BSD_TYPE_NORMAL) {
                            bsd_syslog_debug_debug(BSD_DEFAULT, "wait in bsdSendFile.\n");
                            BSD_BOARD[slotid]->state = BSD_FILE_FINISH;
                            bsd_syslog_debug_debug(BSD_DEFAULT, "slotid = %d, state2 = %d\n",slotid,BSD_BOARD[slotid]->state);
                            timeout.tv_sec = time(0)+160;
                            timeout.tv_nsec = 0;
                            bsd_syslog_debug_debug(BSD_DEFAULT, " call wait condition sendCondition\n");
                            BsdThreadMutexLock(&fileStateMutex[slotid]);
                            if(!BsdWaitThreadConditionTimeout(&fileStateCondition[slotid], &fileStateMutex[slotid], &timeout)) {
                                bsd_syslog_err("wait thread condition timeout in BSDsendFile.\n");
                                ret = BSD_WAIT_THREAD_CONDITION_TIMEOUT;
                            } else if(BSD_BOARD[slotid]->state == BSD_FILE_FAILURE) {
                                bsd_syslog_err("receive BSD_FILE_FAILURE from slot_%d.\n",slotid);
                                ret = BSD_PEER_SAVE_FILE_ERROR;
                                BSD_BOARD[slotid]->state = BSD_FILE_UNKNOWN;
                            } else
                                ret = BSD_SUCCESS;
                            BsdThreadMutexUnlock(&fileStateMutex[slotid]);
                            //bsd_syslog_debug_debug(BSD_DEFAULT, "ret = %d, unlock sendMutex\n");
                        }
                        
                        if(op == BSD_TYPE_CORE) 
                            system("sudo rm -rf /home/admin/proc;");
                        else if((op == BSD_TYPE_BOOT_IMG) 
                            || (op == BSD_TYPE_PATCH)
                            || (op == BSD_TYPE_BLK)) {
                            memset(cmd, 0, BSD_COMMAND_BUF_LEN);
                            sprintf(cmd, "sudo rm %s\n", src_path);
                            system(cmd);
                        }
                        break;
            	    }
        	    }
            }
    	}
    	fclose(fp);
        fp = NULL;
    }
    system("sudo umount /blk");
	return ret;
}



/*****************************************************
** DISCRIPTION:
**          send wtp xml in /blk/wtp and /mnt/wtp
** INPUT:
**          slot id
** OUTPUT:
**          void
** RETURN:
**          0   yes
**          -1   no
** CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-08-18
*****************************************************/
int BSDSendWTP(const unsigned int slotid)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int ret1 = 0;
    int ret2 = 0;
    system("sudo mount /blk");
    ret1 = BSDSendDir(slotid, "/blk/wtp", "/blk/wtp", BSD_TYPE_NORMAL);
    if(ret1 != 0){
        bsd_syslog_err("Error, copy /blk/wtp failed.");
        //return -1;
    }
    ret2 = BSDSendDir(slotid, "/mnt/wtp", "/mnt/wtp", BSD_TYPE_NORMAL);
    if(ret2 != 0){
        bsd_syslog_err("Error, copy /mnt/wtp failed.");
        //return -1;
    }
    if((ret1 == 0) && (ret2 == 0))
        return 0;
    else
        return -1;
}



/*****************************************************
** DISCRIPTION:
**          send ac versions
** INPUT:
**          slot id
** OUTPUT:
**          void
** RETURN:
**          0   yes
**          -1   no
** CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-08-18
*****************************************************/
int BSDSendACVersion(const unsigned int slotid)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    DIR *source = NULL;
    struct dirent *ent=NULL;
    char *source_dir = "/blk";
    char *target_dir = "/blk";
    char name1[PATH_LEN],name2[PATH_LEN];
    system("sudo mount /blk");
    source = opendir(source_dir);
    if(source == NULL){
        bsd_syslog_err("Error, Open dir %s failed.", source_dir);
        //system("sudo umount /blk");
        return -1;
    }
    else{
        while((ent = readdir(source)) != NULL)
        { 
            if( strcmp(ent->d_name, "..")!=0 && strcmp(ent->d_name, ".")!=0)
            {               
                strcpy(name1, "\0");
                strcat(name1, source_dir);
                strcat(name1, "/");
                strcat(name1, ent->d_name);
                strcpy(name2, "\0");
                strcat(name2, target_dir);
                strcat(name2, "/"); 
                strcat(name2, ent->d_name);
                if((ent->d_type == FILE_TYPE) && (strcasestr(name1,".IMG") != NULL)){
                    BSDAddMessageQueue(slotid, name1, name2, BSD_TYPE_NORMAL);
                }
            }
        }
        closedir(source);
    }
    return 0;
}




/*****************************************************
** DISCRIPTION:
**          write file to specific path
** INPUT:
**          receive buffer
**          file length
**          receive over flag:  0 continue, 1 over
**          fp for file pointer
** OUTPUT:
**          null
** RETURN:
**          0   successful
**          1   failed
** CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-08-11
*****************************************************/
int BSDReceiveFile(const char *recvbuf, unsigned int recv_len, unsigned int iflag, FILE **fp)
{	
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    //int testlen= 0;
    //printf("rece_len = %d\n",recv_len);
    if(iflag == 0){       //recv data buf
        fwrite(recvbuf, recv_len, 1, *fp);
        //printf("recv len = %d\n",testlen);
	    //bsd_syslog_debug_debug(BSD_DEFAULT,"recv len = %d\n",testlen);
		return 0;
    }
    else if(iflag == 1){  //save buf to a file
        fwrite(recvbuf, recv_len, 1, *fp);	
        //printf("recv len = %d\n",testlen);
		//bsd_syslog_debug_debug(BSD_DEFAULT,"recv len = %d\n",testlen);
        fclose(*fp);
	    *fp = NULL;
	    //printf("recv over\n");
	    bsd_syslog_debug_debug(BSD_DEFAULT,"Receive file over\n");
		
	    return 0;				
    }
	else{
		bsd_syslog_debug_debug(BSD_DEFAULT,"wrong flag :%d\n", iflag);
		return -1;
	}
    
	return 0;
}



/*****************************************************
** DISCRIPTION:
**          copy file between boards
** INPUT:
**          target slot ID
**          source dir
**          target dir
** OUTPUT:
**          void
** RETURN:
**          0   yes
**          -1   no
** CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-08-16
*****************************************************/
int BSDCopyFile(const unsigned int slotid, const char *source_dir, const char *target_dir, int op)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int ret = 0;
    char cmd[128] = {0};
    char src_dir[PATH_LEN] = {0};
    char src_filename[NAME_LEN] = {0};
    char newSrcPath[PATH_LEN] = {0};
    char newDesPath[PATH_LEN] = {0};
    char md5Result[BSD_COMMAND_BUF_LEN] = {0};
    BSDGetFileDir(source_dir, src_dir);
    BSDGetFileName(source_dir, src_filename);
    if(tarFlag == 1){
        sprintf(cmd, "cd %s; sudo tar -cvf %s.tar %s", (char*)src_dir, src_filename, src_filename);
        //printf("cmd = %s\n",(char*)cmd);
        system(cmd);
	    memset(newSrcPath, 0, PATH_LEN);
	    sprintf(newSrcPath, "%s.tar", source_dir);
	    memset(newDesPath, 0, PATH_LEN);
	    sprintf(newDesPath, "%s.tar", target_dir);
	    //printf("target_dir = %s\n",target_dir);
	    ret = BSDSendFile(slotid, (const char*)newSrcPath, (const char*)newDesPath, op, md5Result);
    }
    else{
        ret = BSDSendDir(slotid, source_dir, target_dir, op);
    }
    return ret;
}



/*****************************************************
** DISCRIPTION:
**          synchronize files to board
** INPUT:
**          void
** OUTPUT:
**          void
** RETURN:
**          0   success
**          -1 fail
**CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-08-18
*****************************************************/
int BSDSynchronizeFilesToBoard(int syn_type, int slot_id) {
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    if(syn_type == BSD_SYNC_AC_VERSION)
        return BSDSendACVersion(slot_id);
    else if(syn_type == BSD_SYNC_WTP)
        return BSDSendWTP(slot_id);
    
    return 0;
}



/*****************************************************
** DISCRIPTION:
**          synchronize files
** INPUT:
**          void
** OUTPUT:
**          void
** RETURN:
**          0   success
**          -1 fail
**CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-08-18
*****************************************************/
int BSDSynchronizeFiles(int syn_type)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    /* get slot_id of all working boards with type MASTER or AC */
    //printf("1\n");
    unsigned int board_on_mask = 0;
	int i = 0;
	int slot_count = 0;
	unsigned int function_type;
	int board_state = 0;
	int is_active_master = 0;
	FILE *fd = NULL;
	char temp_buf[100] = {0};
	char temp_buf2[100] = {0};
	
	strcpy(temp_buf, "/dbm/product");
	
	strcpy(temp_buf2, temp_buf);
	strcat(temp_buf2, "/board_on_mask");
	//printf("2\n");
	fd = fopen(temp_buf2, "r");
	if (fd == NULL){
		bsd_syslog_err("Error:open board_on_mask failed\n");
		return BSD_GET_SLOT_INFORMATION_ERROR;
	}
	fscanf(fd, "%u", &board_on_mask);
	fclose(fd);
    //printf("3\n");
	strcpy(temp_buf2, temp_buf);
	strcat(temp_buf2, "/slotcount");
	fd = fopen(temp_buf2, "r");
	if (fd == NULL){
		bsd_syslog_err("Error:open slot_count failed\n");
		return BSD_GET_SLOT_INFORMATION_ERROR;
	}
	fscanf(fd, "%d", &slot_count);
	fclose(fd);
	//printf("4 slot_count = %d\n",slot_count);
	if (board_on_mask == 0){
		bsd_syslog_err("get wrong board_on_mask\n");
	}
	else{
		for (i=0; i<slot_count; i++){
		    //printf("i = %d\n",i);
			sprintf(temp_buf, "/dbm/product/slot/slot%d/board_state", i+1);
			fd = fopen(temp_buf, "r");
			if (fd == NULL){
				bsd_syslog_err("Error:open %s failed\n", temp_buf);
				return BSD_GET_SLOT_INFORMATION_ERROR;
			}
			fscanf(fd, "%d", &board_state);
			fclose(fd);

            //printf("5\n");
            /* ----------board is on------------ */
			if (board_on_mask & (0x1<<i)){
			    //printf("6 board_state = %d\n",board_state);

			    
				if (board_state <= 1){
					bsd_syslog_debug_debug(BSD_DEFAULT, "slot %d:not work normal\n", i+1);
					continue;
				}
				
				//printf("working slot_id = %d\n",i+1);
				bsd_syslog_debug_debug(BSD_DEFAULT, "slot %d:\n", i+1);
				sprintf(temp_buf, "/dbm/product/slot/slot%d", i+1);

	            
                /* ----------function type----------- */
				strcpy(temp_buf2, temp_buf);
				strcat(temp_buf2, "/function_type");
				fd = fopen(temp_buf2, "r");
				if (fd == NULL){
					bsd_syslog_err("Error:open function_type failed\n");
					return BSD_GET_SLOT_INFORMATION_ERROR;
				}
				fscanf(fd, "%u", &function_type);
				fclose(fd);
				//printf("7 function_type = %d\n",function_type);
				if((syn_type != BSD_SYNC_AC_VERSION) && ((function_type & MASTER_BOARD) == 0) && ((function_type & AC_BOARD) == 0)){
				    bsd_syslog_debug_debug(BSD_DEFAULT, "Function type is not match\n");
				    continue;
				}
                

                /* ---------is active master---------- */
				strcpy(temp_buf2, temp_buf);
				strcat(temp_buf2, "/is_active_master");
				fd = fopen(temp_buf2, "r");
				if (fd == NULL){
					bsd_syslog_err("Error:open is_active_master failed\n");
					return BSD_GET_SLOT_INFORMATION_ERROR;
				}
				if(EOF == fscanf(fd, "%d", &is_active_master)) {
				    bsd_syslog_err("Error:get is_active_master failed\n");
					return BSD_GET_SLOT_INFORMATION_ERROR;
				}
				fclose(fd);
				//printf("8 is_active_master = %d\n", is_active_master);
				if(is_active_master == 1){
				    bsd_syslog_debug_debug(BSD_DEFAULT, "This is an active master board\n");
				    continue;
				}
                
                //printf("right slot_id = %d\n",i+1);
				/* --------Synchronize files to board----------- */
				if(BSDSynchronizeFilesToBoard(syn_type, i+1) != 0){
				    bsd_syslog_err("Error: Copy files to slot%d failed",i+1);
				}
			}
		}
	}
    
    return BSD_SUCCESS;
}



/*****************************************************
** DISCRIPTION:
**          synchronize files version2(with full path name)
** INPUT:
**          void
** OUTPUT:
**          void
** RETURN:
**          0   success
**          -1 fail
**CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-08-18
*****************************************************/
int BSDSynchronizeFilesV2(const char *src_path, const char *des_path, const int op)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    /* get slot_id of all working boards with type MASTER or AC */
    //printf("1\n");
    unsigned int board_on_mask = 0;
	int i = 0;
	int slot_count = 0;
	unsigned int function_type;
	int board_state = 0;
	int is_active_master = 0;
	FILE *fd = NULL;
	char temp_buf[100] = {0};
	char temp_buf2[100] = {0};
	
	strcpy(temp_buf, "/dbm/product");
	
	strcpy(temp_buf2, temp_buf);
	strcat(temp_buf2, "/board_on_mask");
	//printf("2\n");
	fd = fopen(temp_buf2, "r");
	if (fd == NULL){
		bsd_syslog_err("Error:open board_on_mask failed\n");
		return BSD_GET_SLOT_INFORMATION_ERROR;
	}
	fscanf(fd, "%u", &board_on_mask);
	fclose(fd);
    //printf("3\n");
	strcpy(temp_buf2, temp_buf);
	strcat(temp_buf2, "/slotcount");
	fd = fopen(temp_buf2, "r");
	if (fd == NULL){
		bsd_syslog_err("Error:open slot_count failed\n");
		return BSD_GET_SLOT_INFORMATION_ERROR;
	}
	fscanf(fd, "%d", &slot_count);
	fclose(fd);
	//printf("4 slot_count = %d\n",slot_count);
	if (board_on_mask == 0){
		bsd_syslog_err("get wrong board_on_mask\n");
	}
	else{
		for (i=0; i<slot_count; i++){
		    //printf("i = %d\n",i);
			sprintf(temp_buf, "/dbm/product/slot/slot%d/board_state", i+1);
			fd = fopen(temp_buf, "r");
			if (fd == NULL){
				bsd_syslog_err("Error:open %s failed\n", temp_buf);
				return BSD_GET_SLOT_INFORMATION_ERROR;
			}
			fscanf(fd, "%d", &board_state);
			fclose(fd);

            //printf("5\n");
            /* ----------board is on------------ */
			if (board_on_mask & (0x1<<i)){
			    //printf("6 board_state = %d\n",board_state);

			    
				if (board_state <= 1){
					bsd_syslog_debug_debug(BSD_DEFAULT, "slot %d:not work normal\n", i+1);
					continue;
				}
				
				//printf("working slot_id = %d\n",i+1);
				bsd_syslog_debug_debug(BSD_DEFAULT, "slot %d:\n", i+1);
				sprintf(temp_buf, "/dbm/product/slot/slot%d", i+1);

	            
                /* ----------function type----------- */
				strcpy(temp_buf2, temp_buf);
				strcat(temp_buf2, "/function_type");
				fd = fopen(temp_buf2, "r");
				if (fd == NULL){
					bsd_syslog_err("Error:open function_type failed\n");
					return BSD_GET_SLOT_INFORMATION_ERROR;
				}
				fscanf(fd, "%u", &function_type);
				fclose(fd);
				//printf("7 function_type = %d\n",function_type);
				if((op != BSD_TYPE_BOOT_IMG) && ((function_type & MASTER_BOARD) == 0) && ((function_type & AC_BOARD) == 0)){
				    bsd_syslog_debug_debug(BSD_DEFAULT, "Function type is not match\n");
				    continue;
				}
                

                /* ---------is active master---------- */
				strcpy(temp_buf2, temp_buf);
				strcat(temp_buf2, "/is_active_master");
				fd = fopen(temp_buf2, "r");
				if (fd == NULL){
					bsd_syslog_err("Error:open is_active_master failed\n");
					return BSD_GET_SLOT_INFORMATION_ERROR;
				}
				if(EOF == fscanf(fd, "%d", &is_active_master)) {
				    bsd_syslog_err("Error:read is_active_master from file failed.\n");
					return BSD_GET_SLOT_INFORMATION_ERROR;
				}
				fclose(fd);
				//printf("8 is_active_master = %d\n", is_active_master);
				if(is_active_master == 1){
				    bsd_syslog_debug_debug(BSD_DEFAULT, "This is an active master board\n");
				    continue;
				}
                
                //printf("right slot_id = %d\n",i+1);
				/* --------Synchronize files to board----------- */
				if(BSDSendDir(i+1, src_path, des_path, op) != 0){
				    bsd_syslog_err("Error: Copy files to slot%d failed",i+1);
				}
			}
		}
		
	}
    
    return 0;
}




/*****************************************************
** DISCRIPTION:
**          send notify message back
**          tell source slot the file is already writen
** INPUT:
**          source slot ID
** OUTPUT:
**          void
** RETURN:
**          0   success
**          -1 fail
** CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-08-22
*****************************************************/
int BSDSendNotify(int slot_no, int stat, unsigned short event_id, char *md5)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int sd = BSD_BOARD[slot_no]->tipcfd;
    bsd_file_info_t fileInfo;
    memset(&fileInfo, 0, sizeof(bsd_file_info_t));
    fileInfo.file_head.file_state = stat;
    fileInfo.file_head.event_id = event_id;
    fileInfo.slot_no = HOST_SLOT_NO;
    memcpy(fileInfo.file_head.md5Result, md5, strnlen(md5, BSD_PATH_LEN));

    if(0 == wait_for_server(&(BSD_BOARD[slot_no]->tipcaddr.addr.name.name),10000))
    {
		bsd_syslog_debug_debug(BSD_DEFAULT, "******server ok*\n\n");
		//printf("******server ok*\n");
	}
	else
	{			
		bsd_syslog_debug_debug(BSD_DEFAULT, "******server not catch*\n\n");
		//printf("******server not catch*\n");
		return -1;
	}
    
    if (0 > sendto(sd, (char*)&fileInfo, sizeof(fileInfo), 0, (struct sockaddr *)&BSD_BOARD[slot_no]->tipcaddr, sizeof(struct sockaddr)))
    {
        bsd_syslog_err("Send notify Failed.\n");
		return -1;
    }

    return 0;
}



/*****************************************************
** DISCRIPTION:
**          get all alive slot ids
** INPUT:
**          void
** OUTPUT:
**          slot ids
** RETURN:
**          alive slot count
**CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-10-24
*****************************************************/
int BSDGetAliveSlotIDs(int *ID, int op)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    /* get slot_id of all working boards with type MASTER or AC */
    //printf("1\n");
    unsigned int board_on_mask = 0;
	int i = 0;
	int slot_count = 0;
	unsigned int function_type;
	int board_state = 0;
	int is_active_master = 0;
	FILE *fd = NULL;
	char temp_buf[100] = {0};
	char temp_buf2[100] = {0};
	int alive_slot_count = 0;
	
	strcpy(temp_buf, "/dbm/product");
	
	strcpy(temp_buf2, temp_buf);
	strcat(temp_buf2, "/board_on_mask");
	//printf("2\n");
	fd = fopen(temp_buf2, "r");
	if (fd == NULL){
		bsd_syslog_err("Error:open board_on_mask failed\n");
		return BSD_GET_SLOT_INFORMATION_ERROR;
	}
	fscanf(fd, "%u", &board_on_mask);
	fclose(fd);
    //printf("3\n");
	strcpy(temp_buf2, temp_buf);
	strcat(temp_buf2, "/slotcount");
	fd = fopen(temp_buf2, "r");
	if (fd == NULL){
		bsd_syslog_err("Error:open slot_count failed\n");
		return BSD_GET_SLOT_INFORMATION_ERROR;
	}
	fscanf(fd, "%d", &slot_count);
	fclose(fd);
	//printf("4 slot_count = %d\n",slot_count);
	if (board_on_mask == 0){
		bsd_syslog_err("get wrong board_on_mask\n");
	}
	else{
		for (i=0; i<slot_count; i++){
		    //printf("i = %d\n",i);
			sprintf(temp_buf, "/dbm/product/slot/slot%d/board_state", i+1);
			fd = fopen(temp_buf, "r");
			if (fd == NULL){
				bsd_syslog_err("Error:open %s failed\n", temp_buf);
				return BSD_GET_SLOT_INFORMATION_ERROR;
			}
			fscanf(fd, "%d", &board_state);
			fclose(fd);

            //printf("5\n");
            /* ----------board is on------------ */
			if (board_on_mask & (0x1<<i)){
			    //printf("6 board_state = %d\n",board_state);

			    
				if (board_state <= 1){
					bsd_syslog_debug_debug(BSD_BSDINFO, "slot %d:not work normal\n", i+1);
					continue;
				}
				
				//printf("working slot_id = %d\n",i+1);
				bsd_syslog_debug_debug(BSD_BSDINFO, "slot %d:\n", i+1);
				sprintf(temp_buf, "/dbm/product/slot/slot%d", i+1);

	            
                /* ----------function type----------- */
				strcpy(temp_buf2, temp_buf);
				strcat(temp_buf2, "/function_type");
				fd = fopen(temp_buf2, "r");
				if (fd == NULL){
					bsd_syslog_err("Error:open function_type failed\n");
					return BSD_GET_SLOT_INFORMATION_ERROR;
				}
				if(EOF == fscanf(fd, "%u", &function_type)) {
				    bsd_syslog_err("Error:get function_type failed\n");
					return BSD_GET_SLOT_INFORMATION_ERROR;
				}
				fclose(fd);
				//printf("7 function_type = %d\n",function_type);
				if((op == BSD_TYPE_NORMAL) && ((function_type & MASTER_BOARD) == 0) && ((function_type & AC_BOARD) == 0)){
				    bsd_syslog_debug_debug(BSD_BSDINFO, "Function type is not match\n");
				    continue;
				}
                
                /* ---------is active master---------- */
				strcpy(temp_buf2, temp_buf);
				strcat(temp_buf2, "/is_active_master");
				fd = fopen(temp_buf2, "r");
				if (fd == NULL){
					bsd_syslog_err("Error:open is_active_master failed\n");
					return BSD_GET_SLOT_INFORMATION_ERROR;
				}
				if(EOF == fscanf(fd, "%d", &is_active_master)) {
				    bsd_syslog_err("Error:get is_active_master failed\n");
					return BSD_GET_SLOT_INFORMATION_ERROR;
				}
				fclose(fd);
				//printf("8 is_active_master = %d\n", is_active_master);
				if(is_active_master == 1){
				    bsd_syslog_debug_debug(BSD_BSDINFO, "This is an active master board\n");
				    continue;
				}
                
                //printf("right slot_id = %d\n",i+1);
				/* --------Synchronize files to board----------- */
				ID[alive_slot_count++] = i+1;
				
			}
		}
		
	}
    
    return alive_slot_count;
}



/*****************************************************
** DISCRIPTION:
**          send destination path check request
** INPUT:
**          slot ID
**          destination file path
** OUTPUT:
**          void
** RETURN:
**          0   successful
**          other fail
** CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2012-2-16
*****************************************************/
int BSDSendDesPathCheckRequest(const unsigned int slotid, const char *des_path, unsigned short event_id)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
	int sd = BSD_BOARD[slotid]->tipcfd;
	bsd_file_info_t fileInfo;
	memset(&fileInfo, 0, sizeof(bsd_file_info_t));
	fileInfo.slot_no = HOST_SLOT_NO;
	fileInfo.file_head.file_state = BSD_FILE_DES_PATH_CHECK;
	fileInfo.file_head.event_id = event_id;
	
    strncpy((char*)fileInfo.file_head.uchFileName, des_path, PATH_LEN);

    if(0 == wait_for_server(&(BSD_BOARD[slotid]->tipcaddr.addr.name.name),10000))
    {
		bsd_syslog_debug_debug(BSD_DEFAULT, "******server ok*\n\n");
	}
	else
	{			
		bsd_syslog_debug_debug(BSD_DEFAULT, "******server not catch*\n\n");
		return BSD_SERVER_NOT_CATCH;
	}

	if (0 > sendto(sd, (char*)&fileInfo, sizeof(fileInfo), 0, (struct sockaddr*)&BSD_BOARD[slotid]->tipcaddr, sizeof(struct sockaddr)))
	{
        //printf("d\n");
		return BSD_SEND_MESSAGE_ERROR;
    }
	bsd_syslog_debug_debug(BSD_DEFAULT,"send destination path check request to slot%d ok\n",slotid);
    return BSD_SUCCESS;
}



/** 
  * @brief  copy file/folder to other board
  * @param  slotid   
  * @param  src_path   
  * @param  des_path   
  * @param  op   
  * @param  compress_flag   
  * @return  
  * @author  zhangshu
  * @date  2012/06/07
  */
int BSDCopyFilesToBoards(const unsigned int iSlotId, const char *pSrcPath, const char *pDesPath, const int iOption, const int iCompressFlag)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int iReturnValue = BSD_SUCCESS;
    DIR *source = NULL;
    int iOpt = 0;
    char md5Result[BSD_PATH_LEN] = {0};
    system("sudo mount /blk");
    source = opendir(pSrcPath);
    iOpt = iOption;
    if(source == NULL) {
        bsd_syslog_debug_debug(BSD_DEFAULT, "copy single file.\n");
        if(iOption == BSD_TYPE_NORMAL) 
            iOpt = BSD_TYPE_SINGLE;
        
        iReturnValue = BSDSendFile(iSlotId, pSrcPath, pDesPath, iOpt, md5Result);
    } else {//other condition
        bsd_syslog_debug_debug(BSD_DEFAULT, "copy folder.\n");
        tarFlag = iCompressFlag;
        if(iCompressFlag && (iOption != BSD_TYPE_WTP_FOLDER)) {
            iOpt = BSD_TYPE_COMPRESS;
        }
        iReturnValue = BSDCopyFile(iSlotId, pSrcPath, pDesPath, iOpt);
        tarFlag = 0;
        closedir(source);
    }
    
    if((iReturnValue != 0) && (BSD_BOARD[iSlotId]->state != BSD_FILE_UNKNOWN)) 
        BSD_BOARD[iSlotId]->state = BSD_FILE_UNKNOWN;
        
    system("sudo umount /blk");
    return iReturnValue;
}


int BSDCalculateMd5(const char *fileName, char *resultStr)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int iReturnValue = BSD_SUCCESS;
	int md5num=0;
    FILE *fp = 	NULL;
    char cmd[BSD_COMMAND_BUF_LEN] = {0};
    char md5Str[BSD_COMMAND_BUF_LEN] = {0};
    char *ptr = NULL;
    system("sudo mount /blk");
	sprintf(cmd,"sor.sh imgmd5 %s 120", fileName);
getmd5:	fp = popen(cmd, "r");
	if(fp) {
		fgets(md5Str, BSD_COMMAND_BUF_LEN, fp);
		pclose(fp);
    } else {
        bsd_syslog_err("calculate md5 of %s error.\n", fileName);
        iReturnValue = BSD_MD5_ERROR;
    }
    bsd_syslog_debug_debug(BSD_DEFAULT, "md5Str = %s\n", (char*)md5Str);
	if(('\0'==md5Str[0])&&(md5num<2)){
		bsd_syslog_debug_debug(BSD_DEFAULT, "wait for Calculate Md5 again\n");
	    sleep(4);
		md5num++;
		goto getmd5;
	}
	if(('\0'==md5Str[0])&&(2==md5num)){
		system("sudo umount /blk");
		return BSD_MD5_ERROR_THIRD;
	}
    ptr = strchr(md5Str, '=');
    if(ptr && (ptr+=2)) /* skip the blank */
        memcpy(resultStr, ptr, strnlen(ptr, BSD_PATH_LEN));
    else 
        iReturnValue = BSD_MD5_ERROR;
    
    bsd_syslog_debug_debug(BSD_DEFAULT, "md5 result = %s\n", (char*)resultStr);
    
    system("sudo umount /blk");
    return iReturnValue;
}


/** 
  * @brief  new methord for copying files of different types
  * @param  slotid   
  * @param  src_path   
  * @param  des_path   
  * @param  tar   
  * @param  op   
  * @return  
  * @author  zhangshu
  * @date  2012/08/03
  */
int BSDHandleFileOp(const int slotid, const char *src_path,  const char *des_path, const int op, const int eventId)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int iReturnValue = BSD_SUCCESS;
    char newSrcPath[BSD_PATH_LEN] = {0};
    char newDesPath[BSD_PATH_LEN] = {0};
    char fileDir[BSD_PATH_LEN] = {0};
    char fileName[BSD_NAME_LEN] = {0};
    char cmd[BSD_COMMAND_BUF_LEN] = {0};
    char resultStr[BSD_COMMAND_BUF_LEN] = {0};
    switch(op) {
        case BSD_TYPE_BOOT_IMG:
        case BSD_TYPE_PATCH:
        case BSD_TYPE_BLK:
            BSDGetFileName(src_path, fileName);
            if((iReturnValue = BSDCalculateMd5(src_path, resultStr)) != BSD_SUCCESS)
                break;
		    /* modify path */
		    memset(cmd, 0, BSD_COMMAND_BUF_LEN);
		    sprintf(newSrcPath, "/home/admin/%s", fileName);
		    sprintf(cmd, "sudo mount /blk; cp %s %s;", src_path, newSrcPath);
		    system(cmd);
		    strncpy((char*)newDesPath, des_path, BSD_PATH_LEN);
		    break;
        case BSD_TYPE_CORE:
            /* redirect file to /home/admin first */
    	    memcpy((char*)newDesPath, "/home/admin", strlen("/home/admin"));
    	    strcat((char*)newDesPath, des_path);
    	    strncpy((char*)newSrcPath, (char*)newDesPath, BSD_PATH_LEN);
    	    BSDGetFileDir((char*)newDesPath, fileDir);
    	    memset(cmd, 0, BSD_COMMAND_BUF_LEN);
    	    sprintf(cmd, "sudo mkdir -p %s; sudo cat %s > %s;", (char*)fileDir, src_path, (char*)newDesPath);
    	    system(cmd);
            break;
        case BSD_TYPE_WTP:
            if(!BSDGetFileName(src_path, fileName)) {
                if((iReturnValue = BSDCalculateMd5(src_path, resultStr)) != BSD_SUCCESS) {
                    bsd_syslog_err("calculate md5 of %s error.\n", src_path);
                    break;
                } else {
                    strncpy((char*)newSrcPath, src_path, BSD_PATH_LEN);
    	            strncpy((char*)newDesPath, des_path, BSD_PATH_LEN);
    	        }
	        }
	        break;
	    case BSD_TYPE_SINGLE:
	    case BSD_TYPE_FOLDER:
	    case BSD_TYPE_CMD:
	    default:
	        strncpy((char*)newSrcPath, src_path, BSD_PATH_LEN);
	        strncpy((char*)newDesPath, des_path, BSD_PATH_LEN);
	        break;
    }

    bsd_syslog_debug_debug(BSD_DEFAULT, "src_path = %s, des_path = %s, new_src_path = %s, new_des_path = %s\n", src_path,des_path, newSrcPath, newDesPath);
    if((iReturnValue == BSD_SUCCESS) && ((iReturnValue = BSDMemeryCheck(slotid, newSrcPath, des_path, eventId)) == BSD_SUCCESS)) {
        /* check memery, book add, 2012-2-3 */
        if((iReturnValue = BSDDesPathCheck(slotid, des_path, eventId)) == 0) {
            /* send file */
            iReturnValue = BSDSendFile(slotid, (char*)newSrcPath, (char*)newDesPath, op, (char*)resultStr);
        }
    }
    return iReturnValue;
}
