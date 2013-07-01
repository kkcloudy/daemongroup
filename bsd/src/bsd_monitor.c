#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "bsd/bsdpub.h"
#include "bsd.h"
#include "bsd_monitor.h"
#include "bsd_thread.h"
#include "bsd_tipc.h"
#include "bsd_log.h"

extern unsigned short g_unEventId;

/*****************************************************
** DISCRIPTION:
**          manage message queue
** INPUT:
**          void
** OUTPUT:
**          void
** RETURN:
**          void
**CREATOR:sd
**          <zhangshu@autelan.com>
** DATE:
**          2011-08-22
*****************************************************/
void * bsdTipcMonitor(){
	int BSDMsgqID;
	struct BsdMsgQ BSDMsgq;	
	BsdGetMsgQueue(&BSDMsgqID);
	struct timespec timeout;
	int slotid = 0;
	int curr_event_id = 0;
	char md5Result[BSD_PATH_LEN] = {0};
	//printf("BSDMSgqID = %d\n",BSDMsgqID);
    
    while(1) {
        
        timeout.tv_sec = 0;
        timeout.tv_nsec = 0;
        
		memset((char*)&BSDMsgq, 0, sizeof(BSDMsgq));
		if (msgrcv(BSDMsgqID, (struct BsdMsgQ *)&BSDMsgq, sizeof(BSDMsgq.mqinfo), 0, 0) == -1) {
			perror("msgrcv");
			continue;
			//exit(1);
		}
		bsd_syslog_debug_debug(BSD_DEFAULT, "recv msg from msgqueue, op = %d\n", BSDMsgq.mqinfo.op);
        slotid = BSDMsgq.mqinfo.tar_slot_id;
		/* book add msgq op, 2011-11-09 */
        if(BSDMsgq.mqinfo.op != BSD_TYPE_NORMAL) {
            bsd_syslog_err("Error: wrong op type of message in BsdMonitor\n");
            continue;
        }
    	
        bsd_syslog_debug_debug(BSD_DEFAULT, "BSD_BOARD[%d]->state1 = %d\n", slotid, BSD_BOARD[slotid]->state);
		if(BSD_FILE_UNKNOWN == BSD_BOARD[slotid]->state) {
		    if(g_unEventId == MAX_EVENT_ID) {
                g_unEventId = 0;
            }
		    g_unEventId++;
		    curr_event_id = g_unEventId;
		    if(-1 == BSDSendMemeryCheckRequest(slotid, BSDMsgq.mqinfo.src_path, BSDMsgq.mqinfo.tar_path, curr_event_id)){
		        bsd_syslog_err("Error: failed to send memery check to slot_%d\n", slotid);
		        BsdThreadMutexLock(&fileStateMutex[slotid]);
                BSD_BOARD[slotid]->state = BSD_FILE_UNKNOWN;
                BsdThreadMutexUnlock(&fileStateMutex[slotid]);
		        continue;
		    }
		    timeout.tv_sec = time(0) + 100;
            timeout.tv_nsec = 0;
            BsdThreadMutexLock(&fileStateMutex[slotid]);
            BSD_BOARD[slotid]->state = BSD_FILE_MEMERY_CHECK;
            if(!BsdWaitThreadConditionTimeout(&fileStateCondition[slotid], &fileStateMutex[slotid], &timeout)) {
                BSD_BOARD[slotid]->state = BSD_FILE_MEMERY_NOT_ENOUGH;
                bsd_syslog_info("check peer memery over time.\n");    
            }
            BsdThreadMutexUnlock(&fileStateMutex[slotid]);
		} else {
		    bsd_syslog_err("@@@ initial state is wrong.\n");
		    BsdThreadMutexLock(&fileStateMutex[slotid]);
            BSD_BOARD[slotid]->state = BSD_FILE_UNKNOWN;
            BsdThreadMutexUnlock(&fileStateMutex[slotid]);
            continue;
		}
		
		
		bsd_syslog_debug_debug(BSD_DEFAULT, "BSD_BOARD[%d]->state2 = %d\n", slotid, BSD_BOARD[slotid]->state);
		if(BSD_FILE_MEMERY_OK == BSD_BOARD[slotid]->state) {
		    //printf("file state is BSD_FILE_MEMERY_OK\n");
		    if(-1 == BSDSendFile(slotid, BSDMsgq.mqinfo.src_path, BSDMsgq.mqinfo.tar_path, BSDMsgq.mqinfo.op, md5Result)){
                //printf("Error: failed to send file to slot_%d\n", BSDMsgq.mqinfo.tar_slot_id);
                bsd_syslog_err("Error: failed to send file to slot_%d\n", slotid);
                BsdThreadMutexLock(&fileStateMutex[slotid]);
                BSD_BOARD[slotid]->state = BSD_FILE_UNKNOWN;
                BsdThreadMutexUnlock(&fileStateMutex[slotid]);
                continue;
            }
			
            timeout.tv_sec = time(0)+110;
            timeout.tv_nsec = 0;
            BsdThreadMutexLock(&fileStateMutex[slotid]);
            BSD_BOARD[slotid]->state = BSD_FILE_FINISH;
            if(!BsdWaitThreadConditionTimeout(&fileStateCondition[slotid], &fileStateMutex[slotid], &timeout)) {
                BSD_BOARD[slotid]->state = BSD_FILE_UNKNOWN;
                bsd_syslog_err("send file to peer over time.\n");
            }
            BsdThreadMutexUnlock(&fileStateMutex[slotid]);
            continue;
            
		} else if(BSD_FILE_MEMERY_NOT_ENOUGH == BSD_BOARD[slotid]->state) {
		    //printf("file state is BSD_FILE_MEMERY_NOT_ENOUGH\n");
		    bsd_syslog_err("Memery of slot%d is not enough\n", slotid);
		    BsdThreadMutexLock(&fileStateMutex[slotid]);
            BSD_BOARD[slotid]->state = BSD_FILE_UNKNOWN;
            BsdThreadMutexUnlock(&fileStateMutex[slotid]);
		    continue;
		} else {
		    bsd_syslog_err("Memery check with wrong result state %d.\n", BSD_BOARD[slotid]->state);
		    BsdThreadMutexLock(&fileStateMutex[slotid]);
            BSD_BOARD[slotid]->state = BSD_FILE_UNKNOWN;
            BsdThreadMutexUnlock(&fileStateMutex[slotid]);
		    continue;
		}
        
	}
    
    return 0;
    
}



/*****************************************************
** DISCRIPTION:
**          check memery
** INPUT:
**          void
** OUTPUT:
**          void
** RETURN:
**          void
**CREATOR:sd
**          <zhangshu@autelan.com>
** DATE:
**          2012-2-3
*****************************************************/
int BSDMemeryCheck(const unsigned int slotid, const char *src_path, const char *des_path, unsigned short event_id)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int ret = BSD_SUCCESS;
    struct timespec timeout;
    if((ret = BSDSendMemeryCheckRequest(slotid, src_path, des_path, event_id)) != BSD_SUCCESS) {
        bsd_syslog_err("Error: failed to send memery check to slot_%d\n", slotid);
        return ret;
    } else {
	    timeout.tv_sec = time(0)+10;
        timeout.tv_nsec = 0;
        BsdThreadMutexLock(&fileStateMutex[slotid]);
        BSD_BOARD[slotid]->state = BSD_FILE_MEMERY_CHECK;
        if(!BsdWaitThreadConditionTimeout(&fileStateCondition[slotid], &fileStateMutex[slotid], &timeout)) {
            bsd_syslog_info("Wait for memery check response from slot_%d over time, restransmit.\n", slotid);
            BsdThreadMutexUnlock(&fileStateMutex[slotid]);
            /* resend memery check message to peer, zhangshu add 2012-10-2 */
            if((ret = BSDSendMemeryCheckRequest(slotid, src_path, des_path, event_id)) != BSD_SUCCESS) {
                bsd_syslog_err("Error: failed to send memery check to slot_%d\n", slotid);
                return ret;
            } else {
                BsdThreadMutexLock(&fileStateMutex[slotid]);
                BSD_BOARD[slotid]->state = BSD_FILE_MEMERY_CHECK;
                if(!BsdWaitThreadConditionTimeout(&fileStateCondition[slotid], &fileStateMutex[slotid], &timeout)) {
                    ret = BSD_WAIT_THREAD_CONDITION_TIMEOUT;
                }
            }
        }
        BsdThreadMutexUnlock(&fileStateMutex[slotid]);
    }
	
    if(BSD_FILE_MEMERY_NOT_ENOUGH == BSD_BOARD[slotid]->state) {
        bsd_syslog_err("memery is not enough.BSD_BOARD[%d]->state = %d.\n", slotid, BSD_BOARD[slotid]->state);
        ret = BSD_NOT_ENOUGH_MEMERY;
	}
	
    return ret;
}




/*****************************************************
** DISCRIPTION:
**          check destination path
** INPUT:
**          void
** OUTPUT:
**          void
** RETURN:
**          void
**CREATOR:sd
**          <zhangshu@autelan.com>
** DATE:
**          2012-2-16
*****************************************************/
int BSDDesPathCheck(const unsigned int slotid, const char *des_path, unsigned short event_id)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int ret = BSD_SUCCESS;
    struct timespec timeout;
    
    if((ret = BSDSendDesPathCheckRequest(slotid, des_path, event_id)) != BSD_SUCCESS) {
        bsd_syslog_err("Error: failed to send des path check to slot_%d\n", slotid);
        return ret;
    } else {
	    timeout.tv_sec = time(0)+10;
        timeout.tv_nsec = 0;
        BsdThreadMutexLock(&fileStateMutex[slotid]);
        BSD_BOARD[slotid]->state = BSD_FILE_DES_PATH_CHECK;
        if(!BsdWaitThreadConditionTimeout(&fileStateCondition[slotid], &fileStateMutex[slotid], &timeout)) {
            bsd_syslog_info("Wait for path check response from slot_%d over time, restransmit.\n", slotid);
            BsdThreadMutexUnlock(&fileStateMutex[slotid]);
            /* resend path check message to peer, zhangshu add 2012-10-2 */
            if((ret = BSDSendDesPathCheckRequest(slotid, des_path, event_id)) != BSD_SUCCESS) {
                bsd_syslog_err("Error: failed to send des path check to slot_%d\n", slotid);
                return ret;
            } else {
                BsdThreadMutexLock(&fileStateMutex[slotid]);
                BSD_BOARD[slotid]->state = BSD_FILE_DES_PATH_CHECK;
                if(!BsdWaitThreadConditionTimeout(&fileStateCondition[slotid], &fileStateMutex[slotid], &timeout)) {
                    ret = BSD_WAIT_THREAD_CONDITION_TIMEOUT;
                }
            }
        }
        BsdThreadMutexUnlock(&fileStateMutex[slotid]);
    }
	
	
    if(BSD_FILE_DES_PATH_UNA == BSD_BOARD[slotid]->state) {
        bsd_syslog_err("destination path %s illegal.\n", des_path);
		ret = BSD_ILLEGAL_DESTINATION_FILE_PATH;
	}
	
    return ret;
}

