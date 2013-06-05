/*******************************************************************************
Copyright (C) Autelan Technology


This software file is owned and distributed by Autelan Technology 
********************************************************************************


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************
* CWNetwork.h
*
*
* CREATOR:
* autelan.software.wireless-control. team
*
* DESCRIPTION:
* wid module
*
*
*******************************************************************************/

 
#ifndef __CAPWAP_CWNetwork_HEADER__
#define __CAPWAP_CWNetwork_HEADER__

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netdb.h>
#include <poll.h>
#include "CWStevens.h"

typedef int CWSocket;

typedef struct sockaddr_storage CWNetworkLev4Address;

typedef enum {
	CW_IPv6,
	CW_IPv4
} CWNetworkLev3Service;

extern CWNetworkLev3Service gNetworkPreferredFamily;

#define	CW_COPY_NET_ADDR_PTR(addr1, addr2)  	sock_cpy_addr_port(((struct sockaddr*)(addr1)), ((struct sockaddr*)(addr2)))
#define	CW_COPY_NET_ADDR(addr1, addr2)		CW_COPY_NET_ADDR_PTR(&(addr1), &(addr2))

#define CWUseSockNtop(sa, block) 		{ 						\
							char __str[128];			\
							char *str; str = sock_ntop_r(((struct sockaddr*)(sa)), __str);\
							{block}					\
						}

#define CWNetworkRaiseSystemError(error)	{						\
							char buf[256];				\
							if(strerror_r(errno, buf, 256) < 0) {	\
								CWErrorRaise(error, NULL);	\
								return CW_FALSE;		\
							}					\
							CWErrorRaise(error, NULL);		\
							return CW_FALSE;			\
						}

#define		CWNetworkCloseSocket(x)		{ shutdown(SHUT_RDWR, x); close(x); }

int CWNetworkGetAddressSize(CWNetworkLev4Address *addrPtr);
CWBool CWNetworkSendUnsafeConnected(CWSocket sock, const char *buf, int len);
CWBool CWNetworkSendUnsafeUnconnected(CWSocket sock, CWNetworkLev4Address *addrPtr, const char *buf, int len);
CWBool CWNetworkReceiveUnsafe(CWSocket sock, char *buf, int len, int flags, CWNetworkLev4Address *addrPtr, int *readBytesPtr);
CWBool CWNetworkReceiveUnsafeConnected(CWSocket sock, char *buf, int len, int *readBytesPtr);
CWBool CWNetworkInitSocketClient(CWSocket *sockPtr, CWNetworkLev4Address *addrPtr);
CWBool CWNetworkTimedPollRead(CWSocket sock, struct timeval *timeout);
CWBool CWNetworkGetAddressForHost(char *host, CWNetworkLev4Address *addrPtr);


//CWBool CWNetworkInitLib(void);
//CWBool CWNetworkInitSocketServer(CWSocket *sockPtr, int port);
//CWBool CWNetworkSendUnsafeConnected(CWSocket sock, const char *buf, int len);

#endif
