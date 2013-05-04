/**
 * @file bsd_tcp.h
 * @brief define tcp manage functions
 * @author zhangshu
 * @version 1.0.0.0
 * @date 2012-5-24
 */

#ifndef _BSD_TCP_H_
#define _BSD_TCP_H_

#define BSD_TCP_MDU         1448
#define BSD_TCP_BUF_MDU    1447

typedef struct STcpFragMsg{
    char fragFlag;
    char fragBuf[BSD_TCP_BUF_MDU];
}TcpFragmentMessage_t;




/** 
  * @brief  tcp socket node
  * @param  iSocketId   
  * @param  next   
  * @author  zhangshu
  * @date  2012/05/24
  */
struct STcpSocketNode {
    int iSocketId;
    unsigned short unEventId;               //current event id
    char *pFileDataBuf;                     //save every pieces of file data and write it together
    char *pCursor;                          //point to current pointer position
    struct STcpSocketNode *next;
};


/** 
  * @brief  tcp socket list
  * @param  iServerSocketId   
  * @param  iClientSocketCount   
  * @param  clientSocketList 
  * @author  zhangshu
  * @date  2012/05/24
  */
struct STcpClientSockets {
    int iClientSocketCount;
    struct STcpSocketNode *clientSocketList;
};


int bsdInitiateTcpServer(void);

int bsdAcceptTcpConnection(struct STcpClientSockets *clientSockets);

int bsdReceiveTcpMessage(struct STcpClientSockets *clientSockets, struct STcpSocketNode *clientNode);

int bsdTcpCheckDestination(const int iDesIpAddr,const char * pSrcPath,const char * pDesPath, int *pSocketId);

int bsdTcpCopyFile(const int iDesIpAddr, const char *pSrcPath, const char *pDesPath, const int iTarFlag, const int iOperation);

int bsdCloseTcpSocket(int iSocketId);

#endif

