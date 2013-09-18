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
#include "bsd/bsdpub.h"
#include "bsd.h"
#include "bsd_log.h"
#include "bsd_tcp.h"
#include "sysdef/returncode.h"
#include <sys/wait.h>

#define BSD_TCP_SERVER_PORT 44443
#define BSD_TCP_CLIENT_PORT 44444

int g_iLocalTcpSocketId = 0; //global local tcp server socket id
extern unsigned short g_unEventId;
extern DBusConnection *bsd_dbus_notify_connection;


/** 
  * @brief  sent tcp message
  * @param  iPeerSocket   
  * @param  pMsgBuf   
  * @return  
  * @author  zhangshu
  * @date  2012/05/29
  */
static int bsdTcpSendMessage(const int iSocketFd, bsd_file_info_t *pFileInfo)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int iReturnValue = BSD_SUCCESS;
    int iSendLen = 0;
    int iLeftLen = sizeof(bsd_file_info_t);
    char *pSendCursor = (char*)pFileInfo;
    TcpFragmentMessage_t tcpFragMsg;
    int iFragCnt = 0;
    bsd_syslog_debug_debug(BSD_DEFAULT, "socket = %d, file_state = %d\n", iSocketFd, pFileInfo->file_head.file_state);
   
    /* if msg buffer is longer than 1448, need to fragment */
    while(iLeftLen > 0) {
        /* reset memery of tcpFragMsg */
        memset(&tcpFragMsg, 0, sizeof(TcpFragmentMessage_t));

        /* assemble send buffer */
        if(iLeftLen > BSD_TCP_BUF_MDU) {
            tcpFragMsg.fragFlag = 1;    // not last fragment
            memcpy(tcpFragMsg.fragBuf, pSendCursor, BSD_TCP_BUF_MDU);
        } else {
            tcpFragMsg.fragFlag = 0;    // last fragment
            memcpy(tcpFragMsg.fragBuf, pSendCursor, iLeftLen);
        }
        
        /* send */
        iSendLen = send(iSocketFd, (char*)&tcpFragMsg, BSD_TCP_MDU, 0);
        iFragCnt++;
        if(iSendLen <= 0) {
            bsd_syslog_err("Send message to peer failed\n");
            iReturnValue = BSD_SEND_MESSAGE_ERROR;
            break;
        } else {
            pSendCursor += (iSendLen-sizeof(char));
            iLeftLen -= (iSendLen-sizeof(char));
        } 
    }
    return iReturnValue;
}



/** 
  * @brief  receive tcp message
  * @param  iSocketFd   
  * @param  pRecvBuf   
  * @return  
  * @author  zhangshu
  * @date  2012/06/05
  */
static int bsdTcpReceiveMessage(const int iSocketFd, bsd_file_info_t *pFileInfo)
{
     bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int iReturnValue = BSD_SUCCESS;
    int iRemainLen = sizeof(bsd_file_info_t);
    char *recvBufCursor = (char*)pFileInfo;
	int iReceiveBytes = 0;
	int iFragCnt = 0;
	TcpFragmentMessage_t tcpFragMsg;
	memset(&tcpFragMsg, 0, sizeof(TcpFragmentMessage_t));
	
	iReceiveBytes = recv(iSocketFd, (char*)&tcpFragMsg, sizeof(TcpFragmentMessage_t), 0);
    iFragCnt++;
    /* if fragment, need to reassemble packets */
	while((iReceiveBytes > 0) && (tcpFragMsg.fragFlag == 1)) {
	    memcpy(recvBufCursor, tcpFragMsg.fragBuf, iReceiveBytes-sizeof(char));
	    recvBufCursor += (iReceiveBytes-sizeof(char));
	    iRemainLen -= (iReceiveBytes-sizeof(char));
	    memset(&tcpFragMsg, 0, sizeof(TcpFragmentMessage_t));
	    iReceiveBytes = recv(iSocketFd, (char*)&tcpFragMsg, sizeof(TcpFragmentMessage_t), 0);
	    iFragCnt++;
	}
    
	if(iReceiveBytes < 0) {
	    /* receive error */
	    bsd_syslog_err("Error : recv from client %d failed.");
	    iReturnValue = BSD_RECEIVE_MESSAGE_ERROR;
	} else if (iReceiveBytes == 0) {
	    /* connection closed */
	    bsd_syslog_debug_debug(BSD_DEFAULT, "Peer disconnected.\n");
	    iReturnValue = BSD_RECEIVE_MESSAGE_ERROR;
	} else {
	    /* last fragment */
	    if((iRemainLen > 0) && (iRemainLen <= BSD_TCP_BUF_MDU))
	        memcpy(recvBufCursor, tcpFragMsg.fragBuf, iRemainLen);
	    bsd_syslog_debug_debug(BSD_DEFAULT, "total packet len = %d\n", (recvBufCursor-(char*)pFileInfo+iReceiveBytes-sizeof(char)));
	}
	
    return iReturnValue;
}



/** 
  * @brief  init local tcp server
  * @return  BSD_SUCCESS, BSD_ERROR
  * @author  zhangshu
  * @date  2012/05/24
  */
int bsdInitiateTcpServer(void)
{
    bsd_syslog_debug_debug(BSD_BSDINFO, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int iReturnValue = BSD_SUCCESS;
    int iListenSocketFd = 0;
    struct sockaddr_in tcpServerAddr;
    memset(&tcpServerAddr, 0, sizeof(struct sockaddr_in));
    int iReuseAddrFlag = 1;
    
    iListenSocketFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(iListenSocketFd == -1) {
        bsd_syslog_err("Error : Init tcp server socket failed.\n");
        iReturnValue = BSD_INIT_SOCKET_ERROR;
    } else {
        bsd_syslog_debug_debug(BSD_BSDINFO, "server socket id = %d\n", iListenSocketFd);
        tcpServerAddr.sin_family = AF_INET;
        tcpServerAddr.sin_port = htons(BSD_TCP_SERVER_PORT);
        tcpServerAddr.sin_addr.s_addr = 0;
        if(-1 == setsockopt(iListenSocketFd, SOL_SOCKET, SO_REUSEADDR, &iReuseAddrFlag, sizeof(int))) {
            bsd_syslog_err("Error : setsockopt failed.\n");
        }
        
        if ((bind(iListenSocketFd, (struct sockaddr *)&tcpServerAddr, sizeof(struct sockaddr))) < 0) {
    		bsd_syslog_err("Error : Bind tcp server socket failed.\n");
    		iReturnValue = BSD_INIT_SOCKET_ERROR;
    	} else {
    	    g_iLocalTcpSocketId = iListenSocketFd;
            if((listen(iListenSocketFd, 5)) < 0) {
                bsd_syslog_err("Error : Listen tcp server socket failed.\n");
    		    iReturnValue = BSD_INIT_SOCKET_ERROR;
            }  
    	}
    /*	close(iListenSocketFd);*/
    }
    
    return iReturnValue;
}



/** 
  * @brief  accept tcp connect
  * @param  clientSockets   
  * @return  0, success; -1 failed
  * @author  zhangshu
  * @date  2012/05/28
  */
int bsdAcceptTcpConnection(struct STcpClientSockets *clientSockets)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int iReturnValue = BSD_SUCCESS;
    int iSocket = 0;
    struct sockaddr tcpClientAddr;
    int iAddrLen = sizeof(struct sockaddr);
    struct STcpSocketNode *clientSocketNode = NULL;
    clientSocketNode = (struct STcpSocketNode *)malloc(sizeof(struct STcpSocketNode));
    if(clientSocketNode == NULL) {
        bsd_syslog_err("Error : malloc memery for Tcp Socket Node failed.\n");
        iReturnValue = BSD_ESTABLISH_CONNECTION_FAILED;
    } else {
        memset(clientSocketNode, 0, sizeof(struct STcpSocketNode));
        iSocket = accept(g_iLocalTcpSocketId, &tcpClientAddr, (socklen_t*)&iAddrLen);
        if(iSocket == -1) {
            /* error come out */
            bsd_syslog_err("Error : accept connect from client failed.\n");
            BSD_FREE_OBJECT(clientSocketNode);
            iReturnValue = BSD_ESTABLISH_CONNECTION_FAILED;
        } else {
            bsd_syslog_debug_debug(BSD_DEFAULT, "server socket = %d, client socket = %d\n", g_iLocalTcpSocketId, iSocket);
            /* add client into socket list */
            clientSocketNode->iSocketId = iSocket;
            clientSocketNode->next = clientSockets->clientSocketList;
            clientSockets->clientSocketList = clientSocketNode;
            clientSockets->iClientSocketCount++;
        }
    }
    
    return iReturnValue;
}


/** 
  * @brief  delete client node from client list
  * @param  clientSockets   
  * @param  iSocket   
  * @return  0, success; -1 failed
  * @author  zhangshu
  * @date  2012/05/28
  */
static int bsdDeleteClientNode(struct STcpClientSockets * clientSockets, const int iSocket)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int iReturnValue = BSD_SUCCESS;
    struct STcpSocketNode *clientNode = NULL;
    struct STcpSocketNode *tmpNode = NULL;
    if(clientSockets->clientSocketList != NULL && clientSockets->iClientSocketCount != 0) {
        clientNode = clientSockets->clientSocketList;
        if(clientNode->iSocketId == iSocket) {
            clientSockets->clientSocketList = clientNode->next;
            BSD_FREE_OBJECT(clientNode->pFileDataBuf);
            BSD_FREE_OBJECT(clientNode);
        }
        else {
            while(clientNode->next != NULL) {
                tmpNode = clientNode;
                clientNode = tmpNode->next;
                if(clientNode->iSocketId == iSocket) {
                    tmpNode->next = clientNode->next;
                    BSD_FREE_OBJECT(clientNode->pFileDataBuf);
                    BSD_FREE_OBJECT(clientNode);
                    clientSockets->iClientSocketCount--;
                    break;
                }
            }
        }
    }
    return iReturnValue;
}


/** 
  * @brief  
  * @param  clientNode   
  * @param  pRecvFileInfo   
  * @return  
  * @author  zhangshu
  * @date  2012/05/31
  */
static int bsdHandleFileData(struct STcpSocketNode *clientNode, bsd_file_info_t *pRecvFileInfo)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int iReturnValue = BSD_SUCCESS;
    int iBlkFlag = 0;
    char pCmdStr[BSD_COMMAND_BUF_LEN] = {0};
    char pTmpPath[BSD_PATH_LEN] = {0};
    char pFileDir[PATH_LEN] = {0};
	char pFileName[NAME_LEN] = {0};
    FILE * pFilePointer = NULL;
    
    //bsd_syslog_debug_debug(BSD_DEFAULT, "send_len = %d\n", pRecvFileInfo->file_head.send_len);
    //bsd_syslog_debug_debug(BSD_DEFAULT, "already_len = %d\n", pRecvFileInfo->file_head.file_already_len);
    if(clientNode->pCursor == NULL)
        bsd_syslog_debug_debug(BSD_DEFAULT, "cursor is NULL !!!\n");
    if((clientNode->pCursor != NULL) && (pRecvFileInfo->file_head.send_len <= BSD_DATA_LEN)) {
        //bsd_syslog_debug_debug(BSD_DEFAULT, "cursor pos = %d\n", clientNode->pCursor - clientNode->pFileDataBuf);
        memcpy(clientNode->pCursor, (char*)pRecvFileInfo->uchData, pRecvFileInfo->file_head.send_len);
        //bsd_syslog_debug_debug(BSD_DEFAULT, "copy file data successful\n");
	    clientNode->pCursor += pRecvFileInfo->file_head.send_len;

        /* handle last fregment of file buffer */
	    if(pRecvFileInfo->file_head.file_state == BSD_FILE_LAST) {
		    bsd_syslog_debug_debug(BSD_DEFAULT, "file path = %s\n",(char *)pRecvFileInfo->file_head.uchFileName);
		    BSDGetFileDir((const char *)pRecvFileInfo->file_head.uchFileName, pFileDir);
		    BSDGetFileName((const char *)pRecvFileInfo->file_head.uchFileName, pFileName);
            bsd_syslog_debug_debug(BSD_DEFAULT, "file dir = %s\n", (char*)pFileDir);
            bsd_syslog_debug_debug(BSD_DEFAULT, "file name = %s\n", (char*)pFileName);
            /* if copy to /blk, first copy to memery then mv to /blk, book add 2012-2-8 */
            if((strnlen((char*)pFileDir, strlen("/blk")) >= strlen("/blk")) && (memcmp(pFileDir,"/blk", strlen("/blk")) == 0)) {
                strncpy((char*)pTmpPath, "/mnt/", strlen("/mnt/"));
                strncat((char*)pTmpPath, (char*)pFileName, strnlen((char*)pFileName, NAME_LEN));
                iBlkFlag = 1;
                bsd_syslog_debug_debug(BSD_DEFAULT, "pTmpPath = %s\n", (char*)pTmpPath);
            }

            /* if destination file path is not /blk, copy file path to temp path */
            if(!iBlkFlag) {
                memset(pTmpPath, 0, BSD_PATH_LEN);
                strncpy((char*)pTmpPath, (char*)pRecvFileInfo->file_head.uchFileName, BSD_PATH_LEN);
            }
            
		    /* mkdir for file */
		    if(bsdCreateDir(pFileDir) == -1)
		        bsd_syslog_debug_debug(BSD_DEFAULT, "make dir %s failed.\n", (char*)pFileDir);
		    /* write buffer into file */
            pFilePointer = fopen((char*)pTmpPath, "wb");
            if(pFilePointer != NULL) {
                bsd_syslog_debug_debug(BSD_DEFAULT, "open file successful\n");
                fwrite(clientNode->pFileDataBuf, pRecvFileInfo->file_head.file_total_len, 1, pFilePointer);
                fclose(pFilePointer);
                pFilePointer = NULL;
                bsd_syslog_debug_debug(BSD_DEFAULT, "write file successful\n");

                /* if compressed, uncompress file and remove .tar file */
    		    if(pRecvFileInfo->file_head.tar_flag) {
    		        sprintf(pCmdStr, "cd %s; sudo mv %s %s.tar; sudo tar -xvf %s.tar; sudo rm %s.tar",\
    		        (char*)pFileDir, pFileName, pFileName, pFileName, pFileName);
    		        system(pCmdStr);
    		    }
    		    
                /* if destination path is in /blk, call sor.sh to move file from /mnt to /blk */
                if(iBlkFlag) {
                    /* call shell spirit */
                    memset(pCmdStr, 0, BSD_COMMAND_BUF_LEN);
                    sprintf(pCmdStr, "sudo mount /blk; sudo mv /mnt/%s /blk/.; sudo umount /blk; sync;", pFileName);
                    system(pCmdStr);
                }
    		    bsd_syslog_debug_debug(BSD_DEFAULT,"save file successful\n");
            } else {
                bsd_syslog_debug_debug(BSD_DEFAULT,"fopen file %s error.\n",(char*)pTmpPath);
                iReturnValue = BSD_FILE_FAILURE;
            }
		}
    } else {
        iReturnValue = BSD_FILE_FAILURE;
    }
    
    return iReturnValue;
}



/** 
  * @brief  check file path and return check result
  * @param  pFilePath   
  * @return  
  * @author  zhangshu
  * @date  2012/05/31
  */
static int bsdHandleFilePathCheck(char * pFilePath)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int iReturnValue = BSD_FILE_DES_PATH_AVA;
    DIR *pSourceDir = NULL;
    char fileDir[PATH_LEN] = {0};
    
    if(BSDGetFileDir(pFilePath, fileDir) == 0) {
        pSourceDir = opendir(fileDir);
        if(pSourceDir != NULL) {
            closedir(pSourceDir);
            pSourceDir = NULL;
        } else {
            iReturnValue = BSD_FILE_DES_PATH_UNA;
        }
    }
    
    return iReturnValue;
}



/** 
  * @brief  check free memery and return check result
  * @param  uFileSize   
  * @param  pFilePath   
  * @return  
  * @author  zhangshu
  * @date  2012/05/31
  */
static int bsdHandleMemeryQuery(unsigned int uFileSize, char * pFilePath)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int iReturnValue = BSD_FILE_MEMERY_OK;
    int iFuncRet = BSD_SUCCESS;
    unsigned int uFreeMemery = 0;
    int iMemeryType = 0;
    if((pFilePath != NULL) && (strnlen(pFilePath, BSD_PATH_LEN) >= 4)) {
        if(0 == memcmp(pFilePath, "/blk", strlen("/blk")))
            iMemeryType = BSD_BLK_FREE_MEMERY;
        else
            iMemeryType = BSD_SYSTEM_FREE_MEMERY;

        iFuncRet = bsdGetFileSize(&uFreeMemery, pFilePath, iMemeryType);
        if(iFuncRet == BSD_SUCCESS) {
            bsd_syslog_debug_debug(BSD_DEFAULT, "free memery = %u\n", uFreeMemery);
            if(uFreeMemery <= uFileSize)
                iReturnValue = BSD_FILE_MEMERY_NOT_ENOUGH;
        } else {
            iReturnValue = BSD_FILE_MEMERY_NOT_ENOUGH;
        }
    } else {
        iReturnValue = BSD_FILE_MEMERY_NOT_ENOUGH;
    }
    return iReturnValue;
}


int bsd_get_hansiprofile_notity_hmd
(
	bsd_file_info_t *recv_file_info
)
{
	int ret = 0;
	bsd_syslog_debug_debug(BSD_DEFAULT,"recv file name %s\n", 
						recv_file_info->file_head.uchFileName);
	
	ret = system("reloadconfig.sh");
	
	ret = WEXITSTATUS(ret);
	bsd_syslog_debug_debug(BSD_DEFAULT,"reload ret is %d\n",ret);

	ret = system("rm -rf /var/run/config_bak > /dev/null");
	ret = WEXITSTATUS(ret);
	bsd_syslog_debug_debug(BSD_DEFAULT,"rm ret is %d\n",ret);

	ret = system("sudo /opt/bin/vtysh -c  \"write\n\"");	
	ret = WEXITSTATUS(ret);
	bsd_syslog_debug_debug(BSD_DEFAULT,"write ret is %d\n",ret);
	return 0;

}



/** 
  * @brief  
  * @param  iSocketFd   
  * @param  pRecvFileInfo   
  * @return  
  * @author  zhangshu
  * @date  2012/05/31
  */
static int bsdParseTcpMessage(struct STcpSocketNode *clientNode, bsd_file_info_t *pRecvFileInfo)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int iReturnValue = BSD_SUCCESS;
    int iFileState = pRecvFileInfo->file_head.file_state;
    bsd_file_info_t sendFileInfo;
	char temp_buf[100] = {0};
	char temp_buf2[100] = {0};	
	char cmdstr[512]= {0};
	int active_master_slot_id = 0;
	int slot_id = 0;
	char buf2[512] = {0};
	FILE *fp = NULL;
	int ret = 0 ,status = 0 ;
	
    memset(&sendFileInfo, 0, sizeof(bsd_file_info_t));
    bsd_syslog_debug_debug(BSD_DEFAULT, "file state = %d\n", iFileState);
    switch(iFileState) {
        case BSD_FILE_MEMERY_CHECK:
            system("sudo mount /blk");
            sendFileInfo.file_head.file_state = bsdHandleMemeryQuery(pRecvFileInfo->file_head.file_total_len, (char*)pRecvFileInfo->file_head.uchFileName);
            system("sudo umount /blk");
            iReturnValue = bsdTcpSendMessage(clientNode->iSocketId, &sendFileInfo);
            /* if memery check ok, malloc memery to data buffer */
            bsd_syslog_debug_debug(BSD_DEFAULT, "state = %d,  ret = %d\n", sendFileInfo.file_head.file_state, iReturnValue);
            break;
        case BSD_FILE_DES_PATH_CHECK:
            system("sudo mount /blk");
            sendFileInfo.file_head.file_state = bsdHandleFilePathCheck((char*)pRecvFileInfo->file_head.uchFileName);
            system("sudo umount /blk");
            //clientNode->unEventId = pRecvFileInfo->file_head.event_id;
            iReturnValue = bsdTcpSendMessage(clientNode->iSocketId, &sendFileInfo);
            break;
        case BSD_FILE_NORMAL:
            if(clientNode->pFileDataBuf == NULL) {
                clientNode->pFileDataBuf = (char*)malloc(pRecvFileInfo->file_head.file_total_len);
                if(clientNode->pFileDataBuf != NULL) {
                    bsd_syslog_debug_debug(BSD_DEFAULT, "malloc %d bytes memery for FileDataBuffer.\n", pRecvFileInfo->file_head.file_total_len);
                    memset(clientNode->pFileDataBuf, 0, pRecvFileInfo->file_head.file_total_len);
                    clientNode->pCursor = clientNode->pFileDataBuf;
                } else {
                    bsd_syslog_err("malloc memery error!!!\n");
                    iReturnValue = BSD_MALLOC_MEMERY_ERROR;
                }
            }
            iReturnValue = bsdHandleFileData(clientNode, pRecvFileInfo);
            break;
        case BSD_FILE_LAST:
            if(clientNode->pFileDataBuf == NULL) {
                clientNode->pFileDataBuf = (char*)malloc(pRecvFileInfo->file_head.file_total_len);
                if(clientNode->pFileDataBuf != NULL) {
                    bsd_syslog_debug_debug(BSD_DEFAULT, "malloc %d bytes memery for FileDataBuffer.\n", pRecvFileInfo->file_head.file_total_len);
                    memset(clientNode->pFileDataBuf, 0, pRecvFileInfo->file_head.file_total_len);
                    clientNode->pCursor = clientNode->pFileDataBuf;
                } else {
                    bsd_syslog_err("malloc memery error!!!\n");
                    iReturnValue = BSD_MALLOC_MEMERY_ERROR;
                }
            }
            iReturnValue = bsdHandleFileData(clientNode, pRecvFileInfo);
            if(iReturnValue == BSD_SUCCESS)
            {
                sendFileInfo.file_head.file_state = BSD_FILE_FINISH;
				bsd_syslog_debug_debug(BSD_DEFAULT,"bsdParseTcpMessage line %d file name is %s\n",__LINE__,(char *)pRecvFileInfo->file_head.uchFileName);
				bsd_syslog_debug_debug(BSD_DEFAULT,"bsdParseTcpMessage line %d ret is %d\n",__LINE__,strncmp((char *)pRecvFileInfo->file_head.uchFileName,"/var/run/config_bak",strlen("/var/run/config_bak")));
				if(!strncmp((char *)pRecvFileInfo->file_head.uchFileName, 
				   "/var/run/config_bak",
				   strlen("/var/run/config_bak")))
				{
					memset(temp_buf2,0,sizeof(temp_buf2));		//get active_master_slot_id by houxx 2013819
					strcpy(temp_buf,"/dbm/product/active_master_slot_id");
					fp = fopen(temp_buf, "r");
					if (fp == NULL){
	                    bsd_syslog_err("open file error!!!\n");
	                    iReturnValue = BSD_MALLOC_MEMERY_ERROR;
					}
					if(EOF == fscanf(fp, "%d", &active_master_slot_id)) {
	                    bsd_syslog_err("get file error!!!\n");
	                    iReturnValue = BSD_MALLOC_MEMERY_ERROR;
					}
					fclose(fp);
					
					memset(temp_buf2,0,sizeof(temp_buf2));	//get slot_id by houxx 2013819
					strcpy(temp_buf2,"/dbm/local_board/slot_id");
					fp = fopen(temp_buf2, "r");
					if (fp == NULL){
	                    bsd_syslog_err("open file error!!!\n");
	                    iReturnValue = BSD_MALLOC_MEMERY_ERROR;
					}
					if(EOF == fscanf(fp, "%d", &slot_id)) {
	                    bsd_syslog_err("get file error!!!\n");
	                    iReturnValue = BSD_MALLOC_MEMERY_ERROR;
					}
					fclose(fp);	
					bsd_syslog_debug_debug(BSD_DEFAULT,"bsdParseTcpMessage line %d\n",__LINE__);
					sprintf(cmdstr,"copy %d /var/run/config_bak to %d /var/run/config_bak",slot_id,active_master_slot_id);
					sprintf(buf2,"/opt/bin/vtysh -c \"configure terminal\n %s\"\n",cmdstr);
					status = system(buf2);
					ret = WEXITSTATUS(status);
					bsd_syslog_debug_debug(BSD_DEFAULT,"bsdParseTcpMessage line %d ret is %d\n",__LINE__,ret);
					
					status = system("rm -rf /var/run/config_bak > /dev/null");
					ret = WEXITSTATUS(status);
					bsd_syslog_debug_debug(BSD_DEFAULT,"hmd_dbus_conf_sync_to_vrrp_backup ret is %d\n",ret);
	            }
				/*bsd_get_hansiprofile_notity_hmd(pRecvFileInfo);*/
            }
            else
                sendFileInfo.file_head.file_state = iReturnValue;
            iReturnValue = bsdTcpSendMessage(clientNode->iSocketId, &sendFileInfo);
            BSD_FREE_OBJECT(clientNode->pFileDataBuf);
            clientNode->pCursor = NULL;
            clientNode->unEventId = 0;
            break;
        default:
            sendFileInfo.file_head.file_state = BSD_FILE_FAILURE;
            iReturnValue = bsdTcpSendMessage(clientNode->iSocketId, &sendFileInfo);
            BSD_FREE_OBJECT(clientNode->pFileDataBuf);
            clientNode->pCursor = NULL;
            clientNode->unEventId = 0;
            iReturnValue = BSD_UNKNOWN_ERROR;
            break;
    }
    
    if(iReturnValue != BSD_SUCCESS) {
        bsd_syslog_debug_debug(BSD_DEFAULT, "return value = %d\n", iReturnValue);
        BSD_FREE_OBJECT(clientNode->pFileDataBuf);
        clientNode->pCursor = NULL;
        clientNode->unEventId = 0;
    }
    return iReturnValue;
}



/** 
  * @brief  receive message from other peers and handle
  * @param  clientNode   
  * @return  0, success; -1 failed
  * @author  zhangshu
  * @date  2012/05/28
  */
int bsdReceiveTcpMessage(struct STcpClientSockets *clientSockets, struct STcpSocketNode *clientNode)
{   
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int iReturnValue = BSD_SUCCESS;
    bsd_file_info_t bsdFileInfo;
    memset(&bsdFileInfo, 0, sizeof(bsd_file_info_t));
    
    iReturnValue = bsdTcpReceiveMessage(clientNode->iSocketId, &bsdFileInfo);
    bsd_syslog_debug_debug(BSD_DEFAULT, "return value = %d\n", iReturnValue);
    if(iReturnValue == BSD_RECEIVE_MESSAGE_ERROR)
        iReturnValue = bsdDeleteClientNode(clientSockets, clientNode->iSocketId);
    else {
        bsd_syslog_debug_debug(BSD_DEFAULT, "file state = %d\n", bsdFileInfo.file_head.file_state);
	    iReturnValue = bsdParseTcpMessage(clientNode, &bsdFileInfo);   
	}
	
	return iReturnValue;
}


/** 
  * @brief  send a single to peer device
  * @param  iSocketFd   
  * @param  pSrcPath   
  * @param  pDesPath   
  * @param  iCompressFlag   
  * @param  iOperation   
  * @return  
  * @author  zhangshu
  * @date  2012/05/30
  */
static int bsdTcpSendFile(const int iSocketFd, const char *pSrcPath, const char *pDesPath, const int iCompressFlag, const int iOperation)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int iReturnValue = BSD_SUCCESS;
	unsigned int uFileTotalLen = 0;
	unsigned int uFileSendLen = 0;
	unsigned int uFileRemainLen = 0;
	char pCmd[BSD_COMMAND_BUF_LEN] = {0};
	FILE *pFilePointer = NULL;
	bsd_file_info_t sendFileInfo;
	memset(&sendFileInfo, 0, sizeof(bsd_file_info_t));
    bsd_file_info_t recvFileInfo;
	memset(&recvFileInfo, 0, sizeof(bsd_file_info_t));

	if(g_unEventId == MAX_EVENT_ID)
            g_unEventId = 0;
    g_unEventId++;
    sendFileInfo.file_head.event_id = g_unEventId;
	sendFileInfo.file_head.tar_flag = iCompressFlag;
	/*get file name*/
	memset(sendFileInfo.file_head.uchFileName, 0, PATH_LEN);
    memcpy(sendFileInfo.file_head.uchFileName, pDesPath, PATH_LEN);
    
	pFilePointer = fopen(pSrcPath, "rb");
    if(!pFilePointer) {
        bsd_syslog_err("Error : open source file %s failed\n", pSrcPath);
    	iReturnValue = BSD_ILLEGAL_DESTINATION_FILE_PATH;
    } else {
    	fseek(pFilePointer, 0, SEEK_END);
        uFileTotalLen = ftell(pFilePointer);
        fseek(pFilePointer, 0, SEEK_SET);
        sendFileInfo.file_head.file_total_len = uFileTotalLen;
        sendFileInfo.file_head.file_already_len = 0;
        uFileRemainLen = sendFileInfo.file_head.file_total_len;
        
        while(1) {
    		memset(sendFileInfo.uchData, 0, BSD_DATA_LEN);
    	    uFileSendLen = fread(sendFileInfo.uchData, sizeof(char), BSD_DATA_LEN-1, pFilePointer);
    	    sendFileInfo.file_head.send_len = uFileSendLen;
            sendFileInfo.file_head.file_already_len += uFileSendLen;
            bsd_syslog_debug_debug(BSD_DEFAULT, "readLen = %d, send_len = %d, already_len = %d\n", uFileSendLen, sendFileInfo.file_head.send_len, sendFileInfo.file_head.file_already_len);
            uFileRemainLen -= uFileSendLen;
            if(uFileRemainLen > 0) 
        	    sendFileInfo.file_head.file_state = BSD_FILE_NORMAL; 
            else
        	    sendFileInfo.file_head.file_state = BSD_FILE_LAST;
        	
            iReturnValue = bsdTcpSendMessage(iSocketFd, &sendFileInfo);
            if(iReturnValue != BSD_SUCCESS)
                break;
    	    
    	    if(sendFileInfo.file_head.file_state == BSD_FILE_LAST) {
    			bsd_syslog_debug_debug(BSD_DEFAULT, "\tSend file(%s) len(%u) successed.\n", pSrcPath, uFileTotalLen);
                break;
    	    }
    	    
    	    usleep(100);
        }
    }
    
    /* remove compressed file */
    if(iCompressFlag) {
        sprintf(pCmd, "sudo rm %s", pSrcPath);
        system(pCmd);
    }
    
    /* close file */
    if(pFilePointer != NULL) {
        fclose(pFilePointer);
        pFilePointer = NULL;
    }

    /* when finish send file , wait until receive result from peer */
    if(iReturnValue == BSD_SUCCESS) {
        if((iReturnValue = bsdTcpReceiveMessage(iSocketFd, &recvFileInfo)) != BSD_SUCCESS) {
            bsd_syslog_err("Error : receive tcp message failed.\n");
            iReturnValue = BSD_RECEIVE_MESSAGE_ERROR;
        } else if(recvFileInfo.file_head.file_state == BSD_FILE_FAILURE){
            iReturnValue = BSD_PEER_SAVE_FILE_ERROR;
        }
    }
    
    return iReturnValue;
}



/** 
  * @brief  send files in folder with recursion
  * @param  iSocketFd   
  * @param  pSrcPath   
  * @param  pDesPath   
  * @param  iOperation   
  * @return  
  * @author  zhangshu
  * @date  2012/05/30
  */
static int bsdTcpSendFolder(const int iSocketFd, const char *pSrcPath, const char *pDesPath, const int iCompressFlag, const int iOperation)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int iReturnValue = BSD_SUCCESS;
    DIR *pSourceDir = NULL;
    struct dirent *pEnter = NULL;
    char pNewSrcPath[PATH_LEN] = {0};
    char pNewDesPath[PATH_LEN] = {0};
    
    pSourceDir = opendir(pSrcPath);
    if(pSourceDir == NULL) {
        /* this is a file */
        if(0 == access(pSrcPath, F_OK))
            iReturnValue = bsdTcpSendFile(iSocketFd, pSrcPath, pDesPath, iCompressFlag, iOperation); 
        else
            iReturnValue = BSD_ILLEGAL_SOURCE_FILE_PATH;
    } else {
        /* this is a folder, do with recursion */
        while((pEnter = readdir(pSourceDir)) != NULL) { 
            if( strcmp(pEnter->d_name, "..")!=0 && strcmp(pEnter->d_name, ".")!=0) {               
                strcpy(pNewSrcPath, "\0");
                strcat(pNewSrcPath, pSrcPath);
                strcat(pNewSrcPath, "/");
                strcat(pNewSrcPath, pEnter->d_name);
                strcpy(pNewDesPath, "\0");
                strcat(pNewDesPath, pDesPath);
                strcat(pNewDesPath, "/"); 
                strcat(pNewDesPath, pEnter->d_name);
                if(pEnter->d_type == DIR_TPYE)
                    iReturnValue = bsdTcpSendFolder(iSocketFd, pNewSrcPath, pNewDesPath, iCompressFlag, iOperation);
                if(pEnter->d_type == FILE_TYPE)
                    iReturnValue = bsdTcpSendFile(iSocketFd, pNewSrcPath, pNewDesPath, iCompressFlag, iOperation);
            }
        }
        closedir(pSourceDir);
    }
    
    return iReturnValue;
}



/** 
  * @brief  compress file/folder and then send
  * @param  iSocketFd   
  * @param  pSrcPath   
  * @param  pDesPath   
  * @param  iOperation   
  * @return  
  * @author  zhangshu
  * @date  2012/05/30
  */
static int bsdCompressFolder(const int iSocketFd, const char *pSrcPath, const char *pDesPath, const int iOperation)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int iReturnValue = BSD_SUCCESS;
    char pCmd[BSD_COMMAND_BUF_LEN] = {0};
    char pSrcDir[PATH_LEN] = {0};
    char pSrcFileName[NAME_LEN] = {0};

    /* tar file/folder and modify source file path as *.tar */
    BSDGetFileDir(pSrcPath, pSrcDir);
    BSDGetFileName(pDesPath, pSrcFileName);
    sprintf(pCmd, "cd %s; sudo tar -cvf %s.tar %s", (char*)pSrcDir, pSrcFileName, pSrcFileName);
    system(pCmd);
    memset(pCmd, 0, 128);
    sprintf(pCmd, "%s.tar", pSrcPath);
    /* send tar file */
    iReturnValue = bsdTcpSendFile(iSocketFd, (const char*)pCmd, pDesPath, 1, iOperation);
    
    return iReturnValue;
}


/** 
  * @brief  check file path and memery with send & recv message
  * @param  iSocketFd  
  * @param  pDesPath   
  * @param  iCheckType
  * @return  
  * @author  zhangshu
  * @date  2012/05/30
  */
static int bsdTcpCheckFunc(const int iSocketFd, const char *pDesPath, const int iCheckType, bsd_file_info_t *pTcpSendFileInfo)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    bsd_file_info_t tcpRecvFileInfo;
    memset(&tcpRecvFileInfo, 0, sizeof(bsd_file_info_t));
    int iReturnValue = BSD_SUCCESS;
    bsd_syslog_debug_debug(BSD_DEFAULT, "file size = %d\n", pTcpSendFileInfo->file_head.file_total_len);
    pTcpSendFileInfo->file_head.event_id = g_unEventId;
    pTcpSendFileInfo->file_head.file_state = iCheckType;
    memcpy(pTcpSendFileInfo->file_head.uchFileName, pDesPath, strnlen(pDesPath, BSD_PATH_LEN));
    iReturnValue = bsdTcpSendMessage(iSocketFd, pTcpSendFileInfo);
    bsd_syslog_debug_debug(BSD_DEFAULT, "ret = %d\n", iReturnValue);
    if(iReturnValue == BSD_SUCCESS) {
        if((iReturnValue = bsdTcpReceiveMessage(iSocketFd, &tcpRecvFileInfo)) != BSD_SUCCESS) {
            bsd_syslog_err("Error : receive message failed.\n");
            iReturnValue = BSD_RECEIVE_MESSAGE_ERROR;
        } else {
            if(tcpRecvFileInfo.file_head.file_state == BSD_FILE_DES_PATH_UNA)
                iReturnValue = BSD_ILLEGAL_DESTINATION_FILE_PATH;
            else if(tcpRecvFileInfo.file_head.file_state == BSD_FILE_MEMERY_NOT_ENOUGH)
                iReturnValue = BSD_NOT_ENOUGH_MEMERY;
            else
                bsd_syslog_debug_debug(BSD_DEFAULT, "file_state = %d\n", tcpRecvFileInfo.file_head.file_state);            
        }
    }
    
    return iReturnValue;
}



/** 
  * @brief  send client messages
  * @param  iSocketFd   
  * @param  pSrcPath   
  * @param  pDesPath   
  * @return  
  * @author  zhangshu
  * @date  2012/05/30
  */
static int bsdHandleClientMessage(const int iSocketFd, const char *pSrcPath, const char *pDesPath, const int iCompressFlag, const int iOperation)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int iReturnValue = BSD_SUCCESS;
    
    /* send file/folder */
    if(iCompressFlag)
        iReturnValue = bsdCompressFolder(iSocketFd, pSrcPath, pDesPath, iOperation);
    else
        iReturnValue = bsdTcpSendFolder(iSocketFd, pSrcPath, pDesPath, 0, iOperation);
    
    return iReturnValue;
}



/** 
  * @brief  close socket
  * @param  iSocketId   
  * @return  
  * @author  zhangshu
  * @date  2012/06/07
  */
int bsdCloseTcpSocket(int iSocketId)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int iReturnValue = BSD_SUCCESS;
    if(iSocketId > 0) {
        close(iSocketId);
    }
    return iReturnValue;
}



/** 
  * @brief  check free memery and path
  * @param  iDesIpAddr   
  * @param  pSrcPath   
  * @param  pDesPath   
  * @return  
  * @author  zhangshu
  * @date  2012/06/04
  */
int bsdTcpCheckDestination(const int iDesIpAddr, const char *pSrcPath, const char *pDesPath, int *pSocketId)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int iReturnValue = BSD_SUCCESS;
    int iClientSocket = 0;
    int iAddrLen = sizeof(struct sockaddr);
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, iAddrLen);
    bsd_file_info_t tcpSendFileInfo;
    memset(&tcpSendFileInfo, 0, sizeof(bsd_file_info_t));
    unsigned int uSrcFileSize = 0;
    int iReuseAddrFlag = 1;
    
    /* server address init */
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(BSD_TCP_SERVER_PORT);
    memcpy(&(serverAddr.sin_addr),&iDesIpAddr, sizeof(iDesIpAddr));
    
    /* initiate client socket */
    iClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    if(iClientSocket == -1) {
        bsd_syslog_err("Error : Init tcp client socket failed.\n");
        iReturnValue = BSD_INIT_SOCKET_ERROR;
    } else {
        *pSocketId = iClientSocket;
        if(-1 == setsockopt(iClientSocket, SOL_SOCKET, SO_REUSEADDR, &iReuseAddrFlag, sizeof(int))) {
            bsd_syslog_err("Error : setsockopt failed.\n");
        }
        bsd_syslog_debug_debug(BSD_DEFAULT, "client socket id = %d\n", iClientSocket);
        if(connect(iClientSocket, (struct sockaddr *)&serverAddr, iAddrLen) < 0) {
            bsd_syslog_err("Error : connect to peer failed.\n");
            iReturnValue = BSD_ESTABLISH_CONNECTION_FAILED;
        } else {
    	    /* get size of source file/folder */
            iReturnValue = bsdGetFileSize(&uSrcFileSize, pSrcPath, BSD_FILE_FOLDER_SIZE);
            if(iReturnValue == BSD_GET_FILE_SIZE_ERROR) {
                iReturnValue = BSD_ILLEGAL_SOURCE_FILE_PATH;
            } else {
                tcpSendFileInfo.file_head.file_total_len = uSrcFileSize;
                
                /* check destination memery */
                iReturnValue = bsdTcpCheckFunc(iClientSocket, pDesPath, BSD_FILE_MEMERY_CHECK, &tcpSendFileInfo);
                
                /* check destination file path */
                if(iReturnValue == BSD_SUCCESS)
                    iReturnValue = bsdTcpCheckFunc(iClientSocket, pDesPath, BSD_FILE_DES_PATH_CHECK, &tcpSendFileInfo);
                else
                    close(iClientSocket);
            }
        }
    }
    
    return iReturnValue;
}



/** 
  * @brief  
  * @param  iSocketId   
  * @param  pSrcPath   
  * @param  pDesPath   
  * @param  iTarFlag   
  * @param  iOperation   
  * @return  
  * @author  zhangshu
  * @date  2012/06/07
  */
int bsdTcpCopyFile(const int iSocketId, const char *pSrcPath, const char *pDesPath, const int iTarFlag, const int iOperation)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int iReturnValue = BSD_SUCCESS;
    
    bsd_syslog_debug_debug(BSD_DEFAULT, "client socket id = %d\n", iSocketId);
	iReturnValue = bsdHandleClientMessage(iSocketId, pSrcPath, pDesPath, iTarFlag, iOperation);
    
    if(iSocketId != -1) 
        close(iSocketId);
    
    return iReturnValue;
}


